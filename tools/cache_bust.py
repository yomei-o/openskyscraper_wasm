"""Append a content hash to the engine script reference in the generated page.

GitHub Pages serves the HTML with max-age=600 and the .js on its own schedule,
so a browser can end up pairing a freshly fetched page with an engine script it
cached from an earlier build.  When the two disagree about what is exported the
only symptom is a bare TypeError, which says nothing about the real cause.

Usage: cache_bust.py <page.html> <hashed-file>
"""
import hashlib
import re
import sys

page, source = sys.argv[1], sys.argv[2]
stamp = hashlib.sha1(open(source, 'rb').read()).hexdigest()[:10]

html = open(page, encoding='utf-8').read()
# emscripten emits src=openskyscraper.js unquoted after minification
html, n = re.subn(r'(src=)"?(openskyscraper\.js)(\?v=[0-9a-f]+)?"?',
                  lambda m: '%s"%s?v=%s"' % (m.group(1), m.group(2), stamp),
                  html)
if n == 0:
    sys.exit('could not find the engine script reference in %s' % page)
open(page, 'w', encoding='utf-8').write(html)
print('cache-busted %d reference(s) with v=%s' % (n, stamp))
