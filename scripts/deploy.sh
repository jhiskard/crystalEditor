#!/bin/bash
# Deployment script for VTK Workbench

set -euo pipefail

# Configuration
NAMESPACE="${NAMESPACE:-default}"
CONTEXT="${CONTEXT:-}"
IMAGE_TAG="${IMAGE_TAG:-latest}"
DRY_RUN="${DRY_RUN:-false}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking prerequisites..."

    if ! command -v kubectl &> /dev/null; then
        log_error "kubectl is not installed or not in PATH"
        exit 1
    fi

    if ! command -v kustomize &> /dev/null; then
        log_error "kustomize is not installed or not in PATH"
        exit 1
    fi

    # Check kubectl connection
    if ! kubectl cluster-info &> /dev/null; then
        log_error "Cannot connect to Kubernetes cluster"
        exit 1
    fi

    log_info "Prerequisites check passed"
}

# Set kubectl context if provided
set_context() {
    if [[ -n "$CONTEXT" ]]; then
        log_info "Setting kubectl context to: $CONTEXT"
        kubectl config use-context "$CONTEXT"
    fi
}

# Create namespace if it doesn't exist
create_namespace() {
    log_info "Ensuring namespace '$NAMESPACE' exists..."
    kubectl create namespace "$NAMESPACE" --dry-run=client -o yaml | kubectl apply -f -
}

# Deploy application
deploy_app() {
    log_info "Deploying VTK Workbench..."

    # Update image tag in kustomization
    cd k8s
    kustomize edit set image vtk-workbench:$IMAGE_TAG

    # Apply manifests
    if [[ "$DRY_RUN" == "true" ]]; then
        log_info "Dry run mode - showing what would be applied:"
        kustomize build . | kubectl apply --dry-run=client -f - -n "$NAMESPACE"
    else
        kustomize build . | kubectl apply -f - -n "$NAMESPACE"
    fi

    cd ..
}

# Wait for deployment to be ready
wait_for_deployment() {
    if [[ "$DRY_RUN" == "true" ]]; then
        log_info "Skipping deployment wait in dry run mode"
        return
    fi

    log_info "Waiting for deployment to be ready..."
    kubectl rollout status deployment/vtk-workbench-deployment -n "$NAMESPACE" --timeout=300s

    # Check if pods are running
    local ready_pods
    ready_pods=$(kubectl get pods -n "$NAMESPACE" -l app=vtk-workbench --field-selector=status.phase=Running --no-headers | wc -l)
    log_info "Deployment complete. $ready_pods pods running."
}

# Verify deployment
verify_deployment() {
    if [[ "$DRY_RUN" == "true" ]]; then
        log_info "Skipping deployment verification in dry run mode"
        return
    fi

    log_info "Verifying deployment..."

    # Check service endpoints
    kubectl get endpoints vtk-workbench-service -n "$NAMESPACE"

    # Get pod status
    kubectl get pods -n "$NAMESPACE" -l app=vtk-workbench

    # Check ingress
    if kubectl get ingress vtk-workbench-ingress -n "$NAMESPACE" &> /dev/null; then
        kubectl get ingress vtk-workbench-ingress -n "$NAMESPACE"
    fi

    log_info "Deployment verification complete"
}

# Rollback deployment
rollback_deployment() {
    log_warn "Rolling back deployment..."
    kubectl rollout undo deployment/vtk-workbench-deployment -n "$NAMESPACE"
    kubectl rollout status deployment/vtk-workbench-deployment -n "$NAMESPACE" --timeout=300s
    log_info "Rollback complete"
}

# Print usage information
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Deploy VTK Workbench to Kubernetes cluster.

Options:
    -n, --namespace     Target namespace (default: default)
    -c, --context       Kubectl context to use
    -t, --tag           Docker image tag (default: latest)
    -d, --dry-run       Perform dry run without applying changes
    -r, --rollback      Rollback to previous deployment
    -h, --help          Show this help message

Environment Variables:
    NAMESPACE           Target namespace
    CONTEXT             Kubectl context
    IMAGE_TAG           Docker image tag
    DRY_RUN             Dry run mode (true/false)

Examples:
    $0 --namespace production --tag v1.2.3
    $0 --dry-run --context staging
    $0 --rollback --namespace production
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -n|--namespace)
            NAMESPACE="$2"
            shift 2
            ;;
        -c|--context)
            CONTEXT="$2"
            shift 2
            ;;
        -t|--tag)
            IMAGE_TAG="$2"
            shift 2
            ;;
        -d|--dry-run)
            DRY_RUN="true"
            shift
            ;;
        -r|--rollback)
            ROLLBACK="true"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Main execution
main() {
    log_info "Starting VTK Workbench deployment..."
    log_info "Namespace: $NAMESPACE"
    log_info "Image tag: $IMAGE_TAG"
    log_info "Dry run: $DRY_RUN"

    check_prerequisites
    set_context
    create_namespace

    if [[ "${ROLLBACK:-false}" == "true" ]]; then
        rollback_deployment
    else
        deploy_app
        wait_for_deployment
        verify_deployment
    fi

    log_info "Deployment process completed successfully!"
}

# Run main function
main "$@"