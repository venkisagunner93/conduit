function cbuild() {
    docker build -f docker/Dockerfile.conduit -t conduit-dev .
}

function crun() {
    IMAGE_NAME="conduit-dev"
    CONTAINER_NAME="conduit-dev"
    WORKSPACE_DIR="/home/venkat/repos/conduit"

    mkdir -p ~/.claude
    touch ~/.claude.json

    docker run -it --rm \
        --name ${CONTAINER_NAME} \
        -v ${WORKSPACE_DIR}:/home/docker_user/workspace \
        -v ~/.claude:/home/docker_user/.claude \
        -v ~/.claude.json:/home/docker_user/.claude.json \
        --privileged \
        ${IMAGE_NAME}
}

function cexec() {
    CONTAINER_NAME="conduit-dev"
    docker exec -it ${CONTAINER_NAME} /bin/bash
}

function cforge() {
    if [ -f /.dockerenv ]; then
        pip3 install -e ~/workspace/tools/conduit-forge --break-system-packages
    else
        echo "This command must be run inside the conduit-dev container."
    fi
}