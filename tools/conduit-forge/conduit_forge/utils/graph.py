from .package import Package


def build_order(packages: dict[str, Package]) -> list[str]:
    """Topological sort of packages by dependencies."""
    result = []
    visited = set()
    temp = set()

    def visit(name: str):
        if name in temp:
            raise RuntimeError(f"Circular dependency detected involving: {name}")
        if name in visited:
            return

        if name not in packages:
            raise RuntimeError(f"Dependency not found: {name}")

        temp.add(name)
        for dep in packages[name].dependencies:
            visit(dep)
        temp.remove(name)
        visited.add(name)
        result.append(name)

    for name in packages:
        visit(name)

    return result


def compute_levels(packages: dict[str, Package], order: list[str]) -> list[list[str]]:
    """Group packages by dependency level for parallel builds."""
    level = {}

    for name in order:
        pkg = packages[name]
        if not pkg.dependencies:
            level[name] = 0
        else:
            level[name] = max(level[dep] for dep in pkg.dependencies) + 1

    if not level:
        return []

    max_level = max(level.values())
    levels = [[] for _ in range(max_level + 1)]

    for name, lvl in level.items():
        levels[lvl].append(name)

    return levels


def filter_to_target(packages: dict[str, Package], order: list[str], target: str) -> list[str]:
    """Filter order to only include target and its dependencies."""
    if target not in packages:
        raise RuntimeError(f"Package not found: {target}")

    needed = set()

    def collect(name: str):
        if name in needed:
            return
        needed.add(name)
        for dep in packages[name].dependencies:
            collect(dep)

    collect(target)

    return [name for name in order if name in needed]
