# Kubernetes 배포 가이드

## 개요
VTK Workbench를 Kubernetes 클러스터에 배포하기 위한 가이드입니다.

## 사전 준비사항

### 시스템 요구사항
- Docker
- kubectl
- kustomize (선택사항)
- Kubernetes 클러스터 접근 권한

### 의존성
- emsdk 4.0.3 (WebAssembly 컴파일용)
- VTK WASM SDK
- OCCT 7.9.0

## 빌드 및 배포 과정

### 1. 로컬 Docker 빌드 테스트
```bash
cd vtk-workbench-main
npm run build:docker
```

### 2. 개발 환경 테스트
```bash
# 개발용 컨테이너 실행
docker-compose up -d vtk-workbench-dev

# 프로덕션 컨테이너 테스트
docker-compose up -d vtk-workbench
```

### 3. 서버에서 배포 준비
서버에서 수행할 작업들:

```bash
# 1. 리포지토리 클론
git clone <repository-url>
cd vtk-workbench-hskim/vtk-workbench-main

# 2. 스크립트 실행 권한 부여
chmod +x scripts/deploy.sh

# 3. Docker 이미지 빌드
docker build -t vtk-workbench:latest .

# 4. 이미지를 레지스트리에 푸시 (필요시)
docker tag vtk-workbench:latest your-registry/vtk-workbench:latest
docker push your-registry/vtk-workbench:latest
```

### 4. Kubernetes 배포

#### 기본 배포
```bash
kubectl apply -f k8s/
```

#### Kustomize 사용 배포
```bash
kubectl apply -k k8s/
```

#### 배포 스크립트 사용
```bash
# 기본 배포
./scripts/deploy.sh

# 네임스페이스 지정
./scripts/deploy.sh --namespace production --tag v1.0.0

# 드라이 런
./scripts/deploy.sh --dry-run
```

## 파일 구조

```
vtk-workbench-main/
├── Dockerfile              # 프로덕션 빌드용
├── Dockerfile.dev          # 개발용
├── docker-compose.yml      # 로컬 테스트용
├── k8s/                    # Kubernetes 매니페스트
│   ├── deployment.yaml
│   ├── service.yaml
│   ├── configmap.yaml
│   ├── ingress.yaml
│   ├── hpa.yaml
│   └── kustomization.yaml
├── scripts/
│   └── deploy.sh           # 배포 자동화 스크립트
└── .github/workflows/
    └── ci-cd.yml           # GitHub Actions CI/CD
```

## 환경별 설정

### Staging
```bash
./scripts/deploy.sh --namespace staging --context staging-cluster
```

### Production
```bash
./scripts/deploy.sh --namespace production --context prod-cluster --tag v1.0.0
```

## 모니터링 및 확인

### 배포 상태 확인
```bash
kubectl get pods -l app=vtk-workbench
kubectl get services
kubectl get ingress
```

### 로그 확인
```bash
kubectl logs -l app=vtk-workbench --follow
```

### 헬스체크
```bash
curl https://your-domain.com/api/health
```

## 롤백
```bash
# 이전 버전으로 롤백
./scripts/deploy.sh --rollback --namespace production

# 또는 kubectl 직접 사용
kubectl rollout undo deployment/vtk-workbench-deployment
```

## CI/CD 설정

GitHub Actions가 설정되어 있어 자동 배포가 가능합니다:
- `develop` 브랜치 → staging 환경
- `main` 브랜치 → production 환경
- 릴리스 태그 → production 환경

필요한 시크릿:
- `KUBE_CONFIG_STAGING`: staging 클러스터 kubeconfig
- `KUBE_CONFIG_PRODUCTION`: production 클러스터 kubeconfig


---


1. 사전 준비

  필요한 도구들:
  - kubectl 설치 및 클러스터 접근 권한
  - Docker (이미지 빌드용)
  - nginx-ingress-controller (클러스터에 설치되어 있어야 함)

  2. 배포 단계

  단계 1: Docker 이미지 빌드 및 푸시

  # 1. 이미지 빌드
  docker build -t vtk-workbench:latest .

  # 2. 레지스트리에 태그 지정 (환경에 맞게 수정)
  docker tag vtk-workbench:latest your-registry/vtk-workbench:latest

  # 3. 레지스트리에 푸시
  docker push your-registry/vtk-workbench:latest

  단계 2: 네임스페이스 확인/생성

  # pt 네임스페이스 확인
  kubectl get namespace pt

  # 없으면 생성
  kubectl create namespace pt

  단계 3: Kubernetes 배포

  # 전체 배포 (Deployment + Service + Ingress)
  kubectl apply -f k8s/vtk-workbench.yaml

  3. 배포 확인

  상태 확인

  # Pod 상태 확인
  kubectl get pods -n pt

  # 배포 상태 확인  
  kubectl get deployment -n pt

  # 서비스 확인
  kubectl get service -n pt

  # Ingress 확인
  kubectl get ingress -n pt

  로그 확인

  # 실시간 로그 모니터링
  kubectl logs -f deployment/vtk-workbench -n pt

  4. 접속 확인

  배포 완료 후 다음 URL로 접속:
  https://portal-test.edison.re.kr/crystal-editor/

  5. 주요 설정

  현재 설정된 내용:
  - 네임스페이스: pt
  - 리소스: CPU 250m-500m, 메모리 512Mi-1Gi
  - 헬스체크: /api/health 엔드포인트 사용
  - WebAssembly 지원: CORS 및 보안 헤더 설정
  - SSL: 자동 리다이렉트 및 강제 적용

  6. 업데이트 배포

  # 새 이미지로 업데이트
  kubectl set image deployment/vtk-workbench vtk-workbench=your-registry/vtk-workbench:new-tag -n pt

  # 롤아웃 상태 확인
  kubectl rollout status deployment/vtk-workbench -n pt

  7. 롤백

  # 이전 버전으로 롤백
  kubectl rollout undo deployment/vtk-workbench -n pt

  이미 k8s 폴더에 완전한 배포 설정이 준비되어 있어서, Docker 이미지만 빌드하고 kubectl apply 명령으로 바로 배포할 수 있습니다.
