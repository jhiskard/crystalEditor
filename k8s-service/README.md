# Kubernetes 배포 가이드

## 사전 요구사항

1. **Kubernetes 클러스터** 접근 권한
2. **kubectl** 설치 및 설정
3. **pt 네임스페이스** 존재
4. **nginx-ingress-controller** 설치
5. **Docker 이미지** 빌드 및 레지스트리 푸시

## 배포 단계

### 1. 네임스페이스 확인/생성
```bash
# 네임스페이스 확인
kubectl get namespace pt

# 없으면 생성
kubectl create namespace pt
```

### 2. Docker 이미지 빌드 및 푸시
```bash
# 이미지 빌드
docker build -t vtk-workbench:latest .

# 태그 지정 (레지스트리 URL에 따라 변경)
docker tag vtk-workbench:latest your-registry/vtk-workbench:latest

# 레지스트리에 푸시
docker push your-registry/vtk-workbench:latest
```

### 3. YAML 파일 적용
```bash
# 배포 실행
kubectl apply -f k8s/vtk-workbench.yaml

# 또는 네임스페이스 명시적 지정
kubectl apply -f k8s/vtk-workbench.yaml -n pt
```

### 4. 배포 상태 확인
```bash
# Pod 상태 확인
kubectl get pods -n pt

# Deployment 상태 확인
kubectl get deployment -n pt

# Service 확인
kubectl get service -n pt

# Ingress 확인
kubectl get ingress -n pt
```

### 5. 로그 확인
```bash
# Pod 로그 확인
kubectl logs deployment/vtk-workbench -n pt

# 실시간 로그 모니터링
kubectl logs -f deployment/vtk-workbench -n pt
```

## 접속 확인

배포 완료 후 다음 URL로 접속:
- **URL**: https://portal-test.edison.re.kr/crystal-editor/

## 업데이트 배포

### 이미지 업데이트
```bash
# 새 이미지 태그로 배포
kubectl set image deployment/vtk-workbench vtk-workbench=your-registry/vtk-workbench:new-tag -n pt

# 롤아웃 상태 확인
kubectl rollout status deployment/vtk-workbench -n pt
```

### YAML 파일 수정 후 재배포
```bash
kubectl apply -f k8s/vtk-workbench.yaml -n pt
```

## 롤백

```bash
# 이전 버전으로 롤백
kubectl rollout undo deployment/vtk-workbench -n pt

# 특정 리비전으로 롤백
kubectl rollout undo deployment/vtk-workbench --to-revision=2 -n pt
```

## 삭제

```bash
# 전체 리소스 삭제
kubectl delete -f k8s/vtk-workbench.yaml

# 또는 개별 삭제
kubectl delete deployment vtk-workbench -n pt
kubectl delete service vtk-workbench-service -n pt
kubectl delete ingress vtk-workbench-ingress -n pt
```

## 트러블슈팅

### Pod가 시작되지 않는 경우
```bash
# 상세 정보 확인
kubectl describe pod <pod-name> -n pt

# 이벤트 확인
kubectl get events -n pt --sort-by=.metadata.creationTimestamp
```

### Ingress 문제
```bash
# Ingress 상세 정보
kubectl describe ingress vtk-workbench-ingress -n pt

# nginx-ingress 로그 확인
kubectl logs -n ingress-nginx deployment/ingress-nginx-controller
```

### 헬스체크 실패
```bash
# 헬스체크 엔드포인트 직접 테스트
kubectl port-forward deployment/vtk-workbench 3000:3000 -n pt
curl http://localhost:3000/api/health
```

## 환경 변수 및 설정

현재 설정된 환경 변수:
- `NODE_ENV=production`
- `PORT=3000`

추가 환경 변수가 필요한 경우 YAML 파일의 `env` 섹션을 수정하세요.