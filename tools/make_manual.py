#!/usr/bin/env python3
# Makes the manual's PDF and HTML from docs/BASIC_MANUAL.md, with Chrome
# printing the PDF (from pico-os's tools/make_manual.py):
#   python3 tools/make_manual.py [docs/BASIC_MANUAL.md [docs/BASIC_MANUAL.pdf [cover.png]]]
# The HTML goes next to the PDF. The cover picture (in docs/) is optional.
# Needs python3-markdown and google-chrome (or chromium) for printing.
import datetime, html, os, re, shutil, subprocess, sys, tempfile

import markdown

src = sys.argv[1] if len(sys.argv) > 1 else 'docs/BASIC_MANUAL.md'
out = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(src)[0] + '.pdf'
cover_name = sys.argv[3] if len(sys.argv) > 3 else 'none'
text = open(src, encoding='utf-8').read()
title = re.search(r'^# (.+)$', text, re.M).group(1)
body_md = re.sub(r'^# .+\n', '', text, count=1)
# The command reference (tools/make_reference.py) follows the manual.
ref = os.path.join(os.path.dirname(src), 'BASIC_REFERENCE.md')
if os.path.exists(ref) and os.path.abspath(ref) != os.path.abspath(src):
    body_md += '\n\n' + re.sub(r'^# .+\n', '', open(ref, encoding='utf-8').read(), count=1)

md = markdown.Markdown(extensions=['tables', 'fenced_code', 'toc', 'sane_lists', 'attr_list'],
                       extension_configs={'toc': {'toc_depth': '2-3'}})
body = md.convert(body_md)
toc = md.toc
docs = os.path.abspath(os.path.dirname(src))
# Pictures from docs/, as files the browser can open.
body = re.sub(r'<img alt="([^"]*)" src="([^":]+)"',
              lambda m: '<figure><img alt="%s" src="file://%s"' % (m.group(1), os.path.join(docs, m.group(2))),
              body)
body = re.sub(r'(<figure><img[^>]*>)', r'\1</figure>', body)
body = body.replace('<p><figure>', '<figure>').replace('</figure></p>', '</figure>')

css = """
@page { size: A4; margin: 18mm 16mm 18mm 16mm;
        @bottom-center { content: counter(page); font: 9pt sans-serif; color: #666; } }
body { font: 10.5pt/1.45 "DejaVu Sans", "Liberation Sans", sans-serif; color: #111; }
h1.title { font-size: 30pt; margin: 60mm 0 4mm; color: #0b3d91; }
.sub { font-size: 13pt; color: #444; }
.cover img { width: 100%; margin-top: 16mm; image-rendering: pixelated; border: 1px solid #333; }
.contents { page-break-before: always; }
.tochead { font-size: 15pt; font-weight: bold; color: #0b3d91; margin-bottom: 4mm; }
.toc ul { list-style: none; padding-left: 1.2em; }
.toc > ul { padding-left: 0; }
.toc ul ul { columns: 3; column-gap: 6mm; }
.toc ul ul li { break-inside: avoid; }
.toc a { color: #111; text-decoration: none; }
h2 { color: #0b3d91; border-bottom: 2px solid #0b3d91; padding-bottom: 2px; margin-top: 1.6em;
     page-break-before: always; }
h3 { color: #0b3d91; margin-top: 1.3em; page-break-after: avoid; }
table { border-collapse: collapse; width: 100%; margin: .6em 0; font-size: 9.2pt; }
th, td { border: 1px solid #bbb; padding: 3px 6px; vertical-align: top; text-align: left; }
th { background: #e8eef8; }
tr { page-break-inside: avoid; }
code { font: 9pt "DejaVu Sans Mono", monospace; background: #f2f2f2; padding: 0 2px; }
pre { background: #10141c; color: #e6e6e6; padding: 8px 10px; border-radius: 4px;
      font-size: 8.8pt; line-height: 1.35; page-break-inside: avoid; white-space: pre-wrap; }
pre code { background: none; color: inherit; padding: 0; }
figure { margin: .8em 0; text-align: center; page-break-inside: avoid; }
figure img { max-width: 82%; image-rendering: pixelated; border: 1px solid #333; }
"""
cover = cover_name if os.path.exists(os.path.join(docs, cover_name)) else None
page = f"""<!doctype html><html><head><meta charset="utf-8"><title>{html.escape(title)}</title>
<style>{css}</style></head><body>
<div class="cover"><h1 class="title">{html.escape(title)}</h1>
<div class="sub">Pico BASIC for the RP2350 &middot; {datetime.date.today().strftime('%B %d, %Y')}</div>
{f'<img src="file://{os.path.join(docs, cover)}">' if cover else ''}</div>
<div class="contents"><div class="tochead">Contents</div>{toc}</div>
{body}
</body></html>"""

html_out = os.path.splitext(out)[0] + '.html'
open(html_out, 'w', encoding='utf-8').write(page)
print('wrote', html_out)

tmp = tempfile.mkdtemp()
page_path = os.path.join(tmp, 'manual.html')
open(page_path, 'w', encoding='utf-8').write(page)
chrome = shutil.which('google-chrome') or shutil.which('chromium') or shutil.which('chromium-browser')
subprocess.run([chrome, '--headless=new', '--disable-gpu', '--no-pdf-header-footer',
                '--allow-file-access-from-files', '--user-data-dir=' + os.path.join(tmp, 'profile'),
                '--generate-pdf-document-outline', '--print-to-pdf=' + os.path.abspath(out), 'file://' + page_path],
               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
shutil.rmtree(tmp, ignore_errors=True)
print('wrote', out)
