#!/usr/bin/env python3
"""Check that the committed English Qt Linguist catalog is complete."""

from pathlib import Path
from xml.etree import ElementTree


root = Path(__file__).resolve().parents[1]
catalog = root / "translations" / "YandexHomeDesktop_en.ts"
tree = ElementTree.parse(catalog)
missing = []
count = 0

for context in tree.findall("./context"):
    name = context.findtext("name", default="")
    for message in context.findall("message"):
        count += 1
        translation = message.find("translation")
        if (translation is None
                or translation.get("type") == "unfinished"
                or not "".join(translation.itertext()).strip()):
            missing.append((name, message.findtext("source", default="")))

if missing:
    for name, source in missing:
        print(f"Missing English translation in {name}: {source!r}")
    raise SystemExit(1)

print(f"Checked {count} English translations")
