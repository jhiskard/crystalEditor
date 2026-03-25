# VTK Workbench

## Dependencies

- emsdk - 4.0.3
- VTK - kitware/vtk-wasm-sdk:wasm64-threads-v9.4.1-1406-gbbd28af721-20250222 (Docker image with emsdk 3.1.64)
- OCCT - 7.9.0 (built with emsdk 4.0.3)

## Build WebAssembly

### Build: Debug

- WASM related files in "public/wasm" are replaced.

```
npm run build-wasm:debug
```

### Build: Release

- WASM related files in "public/wasm" are replaced.

```
npm run build-wasm:release
```

### Remove build folder

- Remove build foler ("webassembly/build")

```
npm run rm-build
```

### Remove WASM file

- Remove {project_name}.js, {project_name}.wasm and {project_name}.data (if exists)

```
npm run rm-wasm
```



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


---


  0. Docker 이미지 빌드
  docker build -t vtk-workbench:1.0 .
  docker tag vtk-workbench:1.0 10.183.3.3:5000/vtk-workbench:1.0

  1. Docker 이미지를 레지스트리에 푸시

  # 이미지를 프라이빗 레지스트리에 푸시
  docker push 10.183.3.3:5000/vtk-workbench:1.0

  2. Kubernetes 배포 적용

  # k8s 매니페스트 적용
  kubectl apply -f k8s/vtk-workbench.yaml

  3. 배포 상태 확인

  # Pod 상태 확인
  kubectl get pods -n pt

  # Deployment 상태 확인
  kubectl get deployment -n pt

  # Service 확인
  kubectl get service -n pt

  # Ingress 확인
  kubectl get ingress -n pt

  4. 배포 진행 상황 모니터링

  # 실시간 Pod 상태 모니터링
  kubectl get pods -n pt -w

  # 배포 롤아웃 상태 확인
  kubectl rollout status deployment/vtk-workbench -n pt

  # Pod 로그 확인
  kubectl logs -f deployment/vtk-workbench -n pt

  5. 문제 해결 (필요시)

  만약 Pod가 시작되지 않으면:

  # Pod 상세 정보 확인
  kubectl describe pod <pod-name> -n pt

  # 이벤트 확인
  kubectl get events -n pt --sort-by=.metadata.creationTimestamp

  6. 접속 확인

  배포가 완료되면 다음 URL로 접속:
  https://portal-test.edison.re.kr/crystal-editor/

  주요 확인 사항

  - k8s/vtk-workbench.yaml에서 이미지가 10.183.3.3:5000/vtk-workbench:1.0로
  올바르게 설정되어 있음
  - 네임스페이스 pt가 존재하는지 확인
  - nginx-ingress-controller가 클러스터에 설치되어 있는지 확인
  - TLS 인증서 vtk-workbench-tls가 설정되어 있는지 확인

  위 순서대로 진행하면 VTK Workbench가 Kubernetes 클러스터에 배포됩니다.

  ---

  다시 빌드 및 배포

  이제 설정을 수정했으므로 다시 빌드하고 배포해야 합니다:

  # 1. 새로운 버전으로 Docker 이미지 빌드
  docker build -t vtk-workbench:1.1 .
  docker tag vtk-workbench:1.1 10.183.3.3:5000/vtk-workbench:1.1
  docker push 10.183.3.3:5000/vtk-workbench:1.1

  # 2. k8s YAML에서 이미지 태그 업데이트
  # k8s/vtk-workbench.yaml에서 image: 10.183.3.3:5000/vtk-workbench:1.1로 변경

  # 3. Kubernetes 배포 업데이트
  kubectl set image deployment/vtk-workbench vtk-workbench=10.183.3.3:5000/vtk-workbench:1.1 -n pt

  # 4. 롤아웃 상태 확인
  kubectl rollout status deployment/vtk-workbench -n pt

  즉시 테스트 가능한 진단 명령어

  현재 상태를 바로 확인해보려면:

  # 1. 현재 Pod 상태
  kubectl get pods -n pt

  # 2. Pod 로그 확인
  kubectl logs -l app=vtk-workbench -n pt --tail=20

  # 3. 서비스 엔드포인트 확인
  kubectl get endpoints -n pt

  # 4. Ingress 상태
  kubectl describe ingress vtk-workbench-ingress -n pt

⏺ 주요 문제는 Next.js가 /crystal-editor 서브패스를 처리하도록 설정되지 않았던 것입니다.

  basePath와 assetPrefix 설정을 추가했으므로, 새로 빌드하고 배포하면 https://portal-test.edison.re.kr/crystal-editor/ 에서 정상 동작할 것입니다.

  위의 진단 명령어들을 실행해서 현재 상태를 확인해보세요!


-------------

next.config.ts 파일 변경 후 도커 이미지 빌드부터 Kubernetes 배포까지의
  전체 과정:

  1. Docker 이미지 빌드

  # 1. 이미지 빌드 (다단계 빌드)
  docker build -t vtk-workbench:1.3 .

  # 2. 레지스트리에 태그 및 푸시
  docker tag vtk-workbench:1.3 10.183.3.3:5000/vtk-workbench:1.3
  docker push 10.183.3.3:5000/vtk-workbench:1.3

  빌드 과정 (3단계):
  - Stage 1: Emscripten으로 WebAssembly 빌드 (Dockerfile:3-58)
  - Stage 2: Node.js로 Next.js 애플리케이션 빌드 (Dockerfile:60-89)
  - Stage 3: 프로덕션 런타임 환경 구성 (Dockerfile:92-123)

  2. Kubernetes 배포 설정 업데이트

  새 이미지 버전으로 deployment 업데이트:

  # k8s/vtk-workbench.yaml 파일에서 이미지 버전 변경

  k8s/vtk-workbench.yaml:22 라인 수정:
  image: 10.183.3.3:5000/vtk-workbench:1.3  # 1.2 → 1.3으로 변경

  3. Kubernetes 배포 실행

  # 1. 네임스페이스 확인
  kubectl get ns pt

  # 2. 기존 배포 상태 확인
  kubectl get pods -n pt
  kubectl get svc -n pt
  kubectl get ingress -n pt

  # 3. 새 설정 적용
  kubectl apply -f k8s/vtk-workbench.yaml

  # 4. 배포 상태 모니터링
  kubectl rollout status deployment/vtk-workbench -n pt
  kubectl get pods -n pt -w

  # 5. 로그 확인
  kubectl logs -f deployment/vtk-workbench -n pt

  4. 배포 검증

  # 1. 서비스 상태 확인
  kubectl get all -n pt -l app=vtk-workbench

  # 2. ingress 상태 확인
  kubectl describe ingress vtk-workbench-ingress -n pt

  # 3. 애플리케이션 접속 테스트
  curl -I https://portal-test.edison.re.kr/crystal-editor/workbench

  주요 변경사항

  Next.js 설정 변경:
  - basePath: '/crystal-editor' 추가로 모든 asset URL이 올바른 경로로
  생성
  - Kubernetes ingress URL rewriting 제거로 경로 충돌 해결

  예상 결과:
  - ✅ CSS 파일 로드:
  https://portal-test.edison.re.kr/crystal-editor/_next/static/css/...
  - ✅ JavaScript 파일 로드:
  https://portal-test.edison.re.kr/crystal-editor/_next/static/chunks/...
  - ✅ WebAssembly 파일 로드:
  https://portal-test.edison.re.kr/crystal-editor/wasm/VTK-Workbench.js



---------------

기존 도커 이미지들을 모두 제거하고 1.0으로 재빌드하여 배포하는 단계별
  명령어입니다:

  1단계: 기존 로컬 도커 이미지 제거

  # 기존 vtk-workbench 관련 이미지들 확인
  docker images | grep vtk-workbench

  # 모든 vtk-workbench 이미지 제거
  docker rmi $(docker images vtk-workbench* -q) --force

  # 레지스트리 태그된 이미지들도 제거
  docker rmi $(docker images 10.183.3.3:5000/vtk-workbench* -q) --force

  # 사용하지 않는 이미지들 정리
  docker image prune -f

  2단계: 새로운 1.0 버전 빌드

  # WebAssembly + Next.js 전체 빌드
  npm run build:full

  # Docker 이미지 빌드 (1.0 버전으로)
  docker build -t vtk-workbench:1.0 .

  # 레지스트리 태그 생성
  docker tag vtk-workbench:1.0 10.183.3.3:5000/vtk-workbench:1.0

  3단계: 레지스트리에 푸시

  # 이미지를 프라이빗 레지스트리에 푸시
  docker push 10.183.3.3:5000/vtk-workbench:1.0

  4단계: k8s YAML 파일 확인 및 배포

  # k8s/vtk-workbench.yaml에서 이미지 버전이 1.0인지 확인
  # 현재 1.8로 되어 있으니 1.0으로 수정 필요

  # Kubernetes 배포
  kubectl apply -f k8s/vtk-workbench.yaml

  # 배포 상태 모니터링
  kubectl rollout status deployment/vtk-workbench -n pt

  5단계: 배포 확인

  # Pod 상태 확인
  kubectl get pods -n pt -l app=vtk-workbench

  # 서비스 상태 확인
  kubectl get svc -n pt

  # Ingress 상태 확인
  kubectl get ingress -n pt

  # 애플리케이션 로그 확인
  kubectl logs -f deployment/vtk-workbench -n pt

  # 웹사이트 접속 테스트
  curl -I https://crystal-test.edison.re.kr/


---
# 인증서 설치 방법

  kubectl create secret tls vtk-workbench-tls \
    --cert=path/to/edison.re.kr.crt \
    --key=path/to/edison.re.kr.key \
    -n pt
    
#### k8s testbed
kubectl create secret tls vtk-workbench-tls --cert /mnt/luck/ssl/cert.pem --key /mnt/luck/ssl/key.pem -n pt

#### k8s service
kubectl create secret tls vtk-workbench-tls --cert /lustre/service/certs/cert.pem --key /lustre/service/certs/key.pem -n pt



