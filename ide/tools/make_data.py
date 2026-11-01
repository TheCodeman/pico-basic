#!/usr/bin/env python3
"""Makes js/basic-data.js from the manual's A-Z reference: the keywords the
editor colours, and each entry for the help panel.

    python3 ide/tools/make_data.py      (from the pico-basic folder)

Run it again after changing docs/BASIC_REFERENCE.md.
"""
import json
import re
import sys
from pathlib import Path

IDE = Path(__file__).resolve().parent.parent
REF = Path(sys.argv[1]) if len(sys.argv) > 1 else IDE.parent / "docs" / "BASIC_REFERENCE.md"
OUT = IDE / "js" / "basic-data.js"

# Words that are part of other statements and have no entry of their own.
EXTRA_KEYWORDS = """THEN ELSE TO STEP AS APPEND OUTPUT RANDOM BASE FN SEG USING OFF ALL
ACCESS LOCK SHARED READ WRITE CONNECT DISCONNECT PUBLISH SUBSCRIBE UNSUBSCRIBE
PULLUP PULLDOWN SCAN SHOW FILL BRIGHT CLOCK FEED NOSTOP OPEN CLOSE""".split()
OPERATOR_WORDS = {"AND", "OR", "XOR", "NOT", "EQV", "IMP", "MOD"}


def main():
    text = REF.read_text(encoding="utf-8")
    parts = re.split(r"^### (.+?) \{#(ref-[^}]*)\}\s*$", text, flags=re.M)
    entries = []
    for i in range(1, len(parts), 3):
        title, anchor, body = parts[i], parts[i + 1], parts[i + 2].strip()
        if title == "Index by topic":
            continue
        m = re.match(r"\*(\w+)\*\s*·\s*(.*)", body)
        kind = m.group(1) if m else ""
        entries.append({"title": title, "id": anchor, "kind": kind, "body": body})

    keywords = {}
    for e in entries:
        kind = e["kind"].lower() or "statement"
        for word in re.findall(r"[A-Z][A-Z0-9]*\$?", e["title"]):
            if word in OPERATOR_WORDS:
                keywords[word] = "operator"
            elif kind == "function" and word not in keywords:
                keywords[word] = "function"
            elif kind != "function":
                keywords[word] = "statement"
    for word in EXTRA_KEYWORDS:
        keywords.setdefault(word, "statement")
    keywords["MOD"] = "operator"

    data = {"source": "docs/BASIC_REFERENCE.md", "keywords": keywords, "entries": entries}
    OUT.write_text(
        "// Made by tools/make_data.py from docs/BASIC_REFERENCE.md: don't edit.\n"
        "window.BASIC_DATA = " + json.dumps(data, ensure_ascii=False, separators=(",", ":")) + ";\n",
        encoding="utf-8")
    print(f"{OUT}: {len(entries)} entries, {len(keywords)} keywords")


if __name__ == "__main__":
    main()
