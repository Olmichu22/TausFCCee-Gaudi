# Runtime environment. Use with: source env.sh
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$ROOT_DIR/setupKey4Hep.sh"
cd "$ROOT_DIR/k4TauAnalysis" && k4_local_repo
for d in "$ROOT_DIR"/TauTool/install/lib "$ROOT_DIR"/TauTool/install/lib64; do
  [ -d "$d" ] && export LD_LIBRARY_PATH="$d:$LD_LIBRARY_PATH"
done