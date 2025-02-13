import toml
import sys

if len(sys.argv) != 3:
    print("Usage: update_version.py <new_version> <project.toml>")
    sys.exit(1)

new_version = sys.argv[1]
path = sys.argv[2]

data = toml.load(path)
data["project"]["version"] = new_version

with open(path, "w") as f:
    toml.dump(data, f)

print(f"Updated version to {new_version}")
