# VTK Workbench (Crystal Editor)
- 코드 개발자: 김후성 (hskim@kisti.re.kr)

## Dependencies

- emsdk - 4.0.3
- VTK - kitware/vtk-wasm-sdk:wasm64-threads-v9.4.1-1406-gbbd28af721-20250222 (Docker image with emsdk 3.1.64)
- OCCT - 7.9.0 (built with emsdk 4.0.3)

## 배포 방법

#### 1. k8s-testbed

- 소스 코드 다운로드

```
git clone https://yona.edison.re.kr/sghan/vtk-workbench
```

- 변경 파일 복사

```
cd vtk-workbench

sudo mkdir -p /mnt/luck/service/crystal-editor/public
sudo chmod 777 /mnt/luck/service/crystal-editor/public

cp ./public/* /mnt/luck/service/crystal-editor/public/
```

- 도커 이미지 빌드

```
docker build -t vtk-workbench:1.0 . --no-cache

docker tag vtk-workbench:1.0 10.183.3.3:5000/vtk-workbench:1.0
docker push 10.183.3.3:5000/vtk-workbench:1.0
```

- secret 생성

```
kubectl create secret tls vtk-workbench-tls --cert /mnt/luck/ssl/cert.pem --key /mnt/luck/ssl/key.pem -n pt
```

- k8s 배포

```
kubectl apply -f k8s-testbed/vtk-workbench.yaml
```

- 접속 테스트
https://crystal-test.edison.re.kr


---


#### 2. k8s-service

- 소스 코드 다운로드

```
git clone https://yona.edison.re.kr/sghan/vtk-workbench
```

- 변경 파일 복사

```
cd vtk-workbench

sudo mkdir -p /service/crystal-editor/public
sudo chmod 777 /service/crystal-editor/public

cp ./public/* /service/crystal-editor/public/
```

- 도커 이미지 빌드

```
docker build -t vtk-workbench:1.0 . --no-cache

docker tag vtk-workbench:1.0 10.183.3.3:5000/vtk-workbench:1.0
docker push 10.183.3.3:5000/vtk-workbench:1.0
```

- secret 생성

```
kubectl create secret tls vtk-workbench-tls --cert /lustre/service/certs/cert.pem --key /lustre/service/certs/key.pem -n pt
```

- k8s 배포

```
kubectl apply -f k8s-service/vtk-workbench.yaml
```

- 접속 테스트
https://crystal.edison.re.kr


