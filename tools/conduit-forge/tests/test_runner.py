from pathlib import Path
from conduit_forge.utils.package import Package
from conduit_forge.utils.runner import run_in_levels


def _pkg(name, deps=None):
    return Package(name=name, version="0.1.0", path=Path(f"/fake/{name}"), dependencies=deps or [])


def _packages(*specs):
    return {name: _pkg(name, deps) for name, deps in specs}


class TestRunInLevels:
    def test_runs_all_packages(self):
        pkgs = _packages(("a", []), ("b", ["a"]))
        order = ["a", "b"]
        executed = []
        run_in_levels(pkgs, order, lambda p: executed.append(p.name), max_jobs=1)
        assert executed == ["a", "b"]

    def test_returns_failures(self):
        pkgs = _packages(("a", []), ("b", ["a"]))
        order = ["a", "b"]

        def fail_on_b(p):
            if p.name == "b":
                raise RuntimeError("boom")

        failed = run_in_levels(pkgs, order, fail_on_b, max_jobs=1)
        assert failed == ["b"]

    def test_parallel_execution(self):
        pkgs = _packages(("a", []), ("b", []))
        order = ["a", "b"]
        executed = []
        run_in_levels(pkgs, order, lambda p: executed.append(p.name), max_jobs=2)
        assert set(executed) == {"a", "b"}

    def test_empty_order(self):
        failed = run_in_levels({}, [], lambda p: None, max_jobs=1)
        assert failed == []
