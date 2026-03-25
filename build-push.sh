#!/bin/bash
VER=${1:-1.13}  # 인자 없으면 기본값 1.13
REPO="repository.edison.re.kr/vtk-workbench"


echo "Building version: $VER"
sudo docker build -t vtk-workbench:$VER . && \
sudo docker tag vtk-workbench:$VER $REPO:$VER && \
sudo docker push $REPO:$VER

echo "Done! Run on local PC:"
echo "docker pull $REPO:$VER && docker run -it --rm -p 3000:3000 $REPO:$VER"
