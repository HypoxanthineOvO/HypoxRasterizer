from __future__ import annotations

import argparse
import hashlib
import sys
import urllib.error
import urllib.request
from pathlib import Path

from .external_sources import (
    DownloadPlan,
    ExternalSourceManifest,
    ExternalSourceManifestError,
    load_external_sources,
    plan_downloads,
)


def proxy_url_for_port(port: int, approved_ports: list[int]) -> str:
    if port not in approved_ports:
        raise ValueError(f"proxy port {port} is not approved; expected one of {approved_ports}")
    return f"http://127.0.0.1:{port}"


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _has_recorded_sha256(plan: DownloadPlan) -> bool:
    value = plan.sha256.lower()
    return len(value) == 64 and all(ch in "0123456789abcdef" for ch in value)


def _verify_hash_if_recorded(plan: DownloadPlan) -> str:
    if not _has_recorded_sha256(plan):
        return "hash=deferred"
    actual = _sha256(plan.target)
    if actual != plan.sha256.lower():
        raise ValueError(f"{plan.source_id}: sha256 mismatch expected={plan.sha256} actual={actual}")
    return "hash=verified"


def _download(plan: DownloadPlan, proxy_url: str | None, timeout: float, force: bool) -> str:
    plan.target.parent.mkdir(parents=True, exist_ok=True)
    if plan.target.exists() and not force:
        return f"skipped existing {plan.target} {_verify_hash_if_recorded(plan)}"

    opener = urllib.request.build_opener()
    if proxy_url:
        opener = urllib.request.build_opener(
            urllib.request.ProxyHandler({"http": proxy_url, "https": proxy_url})
        )
    with opener.open(plan.url, timeout=timeout) as response:
        plan.target.write_bytes(response.read())

    hash_status = _verify_hash_if_recorded(plan)
    if plan.hash_policy == "record_on_first_download" and not _has_recorded_sha256(plan):
        hash_status = f"hash=record { _sha256(plan.target) }"
    return f"downloaded {plan.target} {hash_status}"


def _format_plan(plan: DownloadPlan, proxy_url: str | None) -> str:
    proxy = proxy_url or "none"
    enabled = "true" if plan.enabled else "false"
    return (
        f"{plan.source_id}: url={plan.url} target={plan.target} tier={plan.tier} "
        f"license={plan.license} format={plan.format} hash_policy={plan.hash_policy} "
        f"sha256={plan.sha256 or 'missing'} conversion={plan.conversion} "
        f"enabled={enabled} proxy={proxy}"
    )


def run(args: argparse.Namespace) -> int:
    manifest: ExternalSourceManifest = load_external_sources(Path(args.manifest))
    proxy_url = proxy_url_for_port(args.proxy_port, manifest.approved_proxy_ports) if args.proxy_port else None
    source_ids = set(args.source) if args.source else None
    plans = plan_downloads(manifest, include_disabled=args.include_disabled, source_ids=source_ids)

    failures = 0
    for plan in plans:
        print(_format_plan(plan, proxy_url), flush=True)
        if args.dry_run:
            continue
        try:
            print(_download(plan, proxy_url, args.timeout, args.force), flush=True)
        except (OSError, ValueError, urllib.error.URLError) as exc:
            failures += 1
            print(f"failed {plan.source_id}: {exc}", file=sys.stderr, flush=True)

    print(f"summary: planned={len(plans)} failed={failures} dry_run={str(args.dry_run).lower()}")
    return 1 if failures else 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Fetch external validation scene assets.")
    parser.add_argument("--manifest", default="validation/external_sources.yaml")
    parser.add_argument("--source", action="append", help="source id to include; can be repeated")
    parser.add_argument("--include-disabled", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--proxy-port", type=int)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--timeout", type=float, default=60.0)
    try:
        return run(parser.parse_args(argv))
    except (ExternalSourceManifestError, ValueError) as exc:
        print(f"fetch-scenes error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
