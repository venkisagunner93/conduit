function cbuild() {
    docker build -f docker/Dockerfile.conduit -t conduit-dev .
}

function crun() {
    IMAGE_NAME="conduit-dev"
    CONTAINER_NAME="conduit-dev"
    WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
    if [ ! -f /.dockerenv ]; then
        echo "This command must be run inside the conduit-dev container."
        return 1
    fi

    case "${1:-install}" in
        install)
            pip3 install -e ~/workspace/tools/conduit-forge --break-system-packages
            ;;
        test)
            pip3 install -e "~/workspace/tools/conduit-forge[test]" --break-system-packages 2>/dev/null
            pytest ~/workspace/tools/conduit-forge/tests/ -v
            ;;
        *)
            echo "Usage: cforge [install|test]"
            return 1
            ;;
    esac
}