import pytest
from pathlib import Path
from conduit_forge.utils.package import Package
from conduit_forge.utils.graph import build_order, compute_levels, filter_to_target


def _pkg(name, deps=None):
    return Package(name=name, version="0.1.0", path=Path(f"/fake/{name}"), dependencies=deps or [])


def _packages(*specs):
    """Build a packages dict from (name, [deps]) tuples."""
    return {name: _pkg(name, deps) for name, deps in specs}


class TestBuildOrder:
    def test_single_package(self):
        pkgs = _packages(("a", []))
        assert build_order(pkgs) == ["a"]

    def test_linear_chain(self):
        pkgs = _packages(("a", []), ("b", ["a"]), ("c", ["b"]))
        order = build_order(pkgs)
        assert order.index("a") < order.index("b") < order.index("c")

    def test_diamond_dependency(self):
        pkgs = _packages(("a", []), ("b", ["a"]), ("c", ["a"]), ("d", ["b", "c"]))
        order = build_order(pkgs)
        assert order.index("a") < order.index("b")
        assert order.index("a") < order.index("c")
        assert order.index("b") < order.index("d")
        assert order.index("c") < order.index("d")

    def test_circular_dependency_raises(self):
        pkgs = _packages(("a", ["b"]), ("b", ["a"]))
        with pytest.raises(RuntimeError, match="Circular dependency"):
            build_order(pkgs)

    def test_missing_dependency_raises(self):
        pkgs = _packages(("a", ["missing"]))
        with pytest.raises(RuntimeError, match="Dependency not found"):
            build_order(pkgs)

    def test_no_packages(self):
        assert build_order({}) == []


class TestComputeLevels:
    def test_single_level(self):
        pkgs = _packages(("a", []), ("b", []))
        order = build_order(pkgs)
        levels = compute_levels(pkgs, order)
        assert levels == [["a", "b"]] or levels == [["b", "a"]]

    def test_two_levels(self):
        pkgs = _packages(("a", []), ("b", ["a"]))
        order = build_order(pkgs)
        levels = compute_levels(pkgs, order)
        assert len(levels) == 2
        assert levels[0] == ["a"]
        assert levels[1] == ["b"]

    def test_three_levels(self):
        pkgs = _packages(("a", []), ("b", ["a"]), ("c", ["b"]))
        order = build_order(pkgs)
        levels = compute_levels(pkgs, order)
        assert len(levels) == 3
        assert levels[0] == ["a"]
        assert levels[1] == ["b"]
        assert levels[2] == ["c"]

    def test_empty(self):
        assert compute_levels({}, []) == []


class TestFilterToTarget:
    def test_target_with_deps(self):
        pkgs = _packages(("a", []), ("b", ["a"]), ("c", ["b"]), ("d", []))
        order = build_order(pkgs)
        filtered = filter_to_target(pkgs, order, "c")
        assert "a" in filtered
        assert "b" in filtered
        assert "c" in filtered
        assert "d" not in filtered

    def test_target_no_deps(self):
        pkgs = _packages(("a", []), ("b", []))
        order = build_order(pkgs)
        filtered = filter_to_target(pkgs, order, "a")
        assert filtered == ["a"]

    def test_target_not_found_raises(self):
        pkgs = _packages(("a", []))
        order = build_order(pkgs)
        with pytest.raises(RuntimeError, match="Package not found"):
            filter_to_target(pkgs, order, "missing")
