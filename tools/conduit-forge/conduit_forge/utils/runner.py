"""Parallel task runner."""

from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Callable, TypeVar

from .package import Package
from .graph import compute_levels

T = TypeVar("T")


def run_in_levels(
    packages: dict[str, Package],
    order: list[str],
    task: Callable[[Package], T],
    max_jobs: int,
    action: str = "Processing",
) -> list[str]:
    """
    Run a task for each package, respecting dependency levels.

    Returns list of failed package names.
    """
    levels = compute_levels(packages, order)
    failed = []

    for i, level in enumerate(levels):
        print(f"=== Level {i}: {level} ===")

        if max_jobs == 1:
            for name in level:
                print(f"{action} {name}...")
                try:
                    task(packages[name])
                    print(f"Done: {name}")
                except Exception as e:
                    print(f"Failed: {name} - {e}")
                    failed.append(name)
        else:
            with ThreadPoolExecutor(max_workers=max_jobs) as pool:
                futures = {pool.submit(task, packages[name]): name for name in level}

                for future in as_completed(futures):
                    name = futures[future]
                    try:
                        future.result()
                        print(f"Done: {name}")
                    except Exception as e:
                        print(f"Failed: {name} - {e}")
                        failed.append(name)

    return failed
