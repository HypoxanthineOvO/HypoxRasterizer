from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


class ExternalSourceManifestError(ValueError):
    pass


@dataclass(frozen=True)
class ExternalSource:
    source_id: str
    url: str
    license: str
    tier: str
    target: Path
    format: str
    sha256: str
    hash_policy: str
    conversion: str
    enabled: bool
    source_page: str = ""
    notes: str = ""


@dataclass(frozen=True)
class ExternalSourceManifest:
    version: int
    approved_proxy_ports: list[int]
    sources: list[ExternalSource]


@dataclass(frozen=True)
class DownloadPlan:
    source_id: str
    url: str
    license: str
    tier: str
    target: Path
    format: str
    sha256: str
    hash_policy: str
    conversion: str
    enabled: bool


def _strip_comment(line: str) -> str:
    in_quote = False
    quote = ""
    for idx, char in enumerate(line):
        if char in {"'", '"'}:
            if not in_quote:
                in_quote = True
                quote = char
            elif quote == char:
                in_quote = False
        if char == "#" and not in_quote:
            return line[:idx]
    return line


def _value(raw: str) -> str:
    value = raw.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in {"'", '"'}:
        return value[1:-1]
    return value


def _list(raw: str) -> list[str]:
    value = raw.strip()
    if value.startswith("[") and value.endswith("]"):
        value = value[1:-1]
    if not value:
        return []
    return [_value(item) for item in value.split(",") if _value(item)]


def _bool(raw: str) -> bool:
    return raw.strip().lower() in {"true", "yes", "1"}


def _parse_manifest(path: Path) -> tuple[int, list[int], list[dict[str, str]]]:
    version = 1
    proxy_ports: list[int] = []
    raw_sources: list[dict[str, str]] = []
    current: dict[str, str] | None = None
    section = "root"

    for raw in path.read_text(encoding="utf-8").splitlines():
        line = _strip_comment(raw).strip()
        if not line:
            continue
        if line == "proxy:":
            section = "proxy"
            current = None
            continue
        if line == "sources:":
            section = "sources"
            current = None
            continue

        if section == "root":
            if ":" not in line:
                continue
            key, raw_value = [part.strip() for part in line.split(":", 1)]
            if key == "version":
                version = int(_value(raw_value))
            continue

        if section == "proxy":
            if ":" not in line:
                continue
            key, raw_value = [part.strip() for part in line.split(":", 1)]
            if key == "approved_ports":
                proxy_ports = [int(item) for item in _list(raw_value)]
            continue

        if section == "sources":
            if line.startswith("- "):
                current = {}
                raw_sources.append(current)
                line = line[2:].strip()
                if not line:
                    continue
            if current is None or ":" not in line:
                continue
            key, raw_value = [part.strip() for part in line.split(":", 1)]
            current[key] = _value(raw_value)

    return version, proxy_ports, raw_sources


def _require(raw: dict[str, str], key: str, source_id: str) -> str:
    value = raw.get(key, "")
    if not value:
        raise ExternalSourceManifestError(f"external source '{source_id}' is missing required field '{key}'")
    return value


def load_external_sources(path: Path | str) -> ExternalSourceManifest:
    manifest_path = Path(path)
    if not manifest_path.exists():
        raise ExternalSourceManifestError(f"external source manifest does not exist: {manifest_path}")

    version, proxy_ports, raw_sources = _parse_manifest(manifest_path)
    sources: list[ExternalSource] = []
    for raw in raw_sources:
        source_id = _require(raw, "id", "<unknown>")
        sha256 = raw.get("sha256", "")
        hash_policy = _require(raw, "hash_policy", source_id)
        if not sha256 and not hash_policy:
            raise ExternalSourceManifestError(f"external source '{source_id}' needs sha256 or hash_policy")
        sources.append(
            ExternalSource(
                source_id=source_id,
                url=_require(raw, "url", source_id),
                license=_require(raw, "license", source_id),
                tier=_require(raw, "tier", source_id),
                target=Path(_require(raw, "target", source_id)),
                format=_require(raw, "format", source_id),
                sha256=sha256,
                hash_policy=hash_policy,
                conversion=_require(raw, "conversion", source_id),
                enabled=_bool(raw.get("enabled", "false")),
                source_page=raw.get("source_page", ""),
                notes=raw.get("notes", ""),
            )
        )

    if not sources:
        raise ExternalSourceManifestError(f"external source manifest has no sources: {manifest_path}")
    return ExternalSourceManifest(version=version, approved_proxy_ports=proxy_ports, sources=sources)


def plan_downloads(
    manifest: ExternalSourceManifest,
    include_disabled: bool = False,
    source_ids: set[str] | None = None,
) -> list[DownloadPlan]:
    plans: list[DownloadPlan] = []
    for source in manifest.sources:
        if source_ids and source.source_id not in source_ids:
            continue
        if not source.enabled and not include_disabled:
            continue
        plans.append(
            DownloadPlan(
                source_id=source.source_id,
                url=source.url,
                license=source.license,
                tier=source.tier,
                target=source.target,
                format=source.format,
                sha256=source.sha256,
                hash_policy=source.hash_policy,
                conversion=source.conversion,
                enabled=source.enabled,
            )
        )
    return plans
