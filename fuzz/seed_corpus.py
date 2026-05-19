#!/usr/bin/env python3
"""Generate seed corpora for SpookyView fuzz harnesses."""
import os, sys

base = os.path.dirname(os.path.abspath(__file__))

seeds_url = {
    'good': 'https://updates.tyndomyn.net/foo',
    'case': 'HTTPS://Updates.Tyndomyn.NET/x',
    'http': 'http://updates.tyndomyn.net/x',
    'js':   'javascript:alert(1)',
    'attacker': 'https://attacker.example.com/',
    'short':  'h',
    'long':   'https://updates.tyndomyn.net/' + 'A'*1000,
    'file':   'file:///c:/windows/system32',
    'unc':    '\\\\attacker\\share\\evil.exe',
}
for n, s in seeds_url.items():
    with open(os.path.join(base, 'corpus/url_allowlist', n), 'wb') as f:
        f.write(s.encode('utf-16-le'))

seeds_json = {
    'good':      b'{"update_available":true,"version":"1.2.3","download_url":"https://updates.tyndomyn.net/x","message":"hi"}',
    'false':     b'{"update_available":false,"version":"0","download_url":"","message":""}',
    'type_int':  b'{"update_available":1,"version":"1","download_url":"","message":""}',
    'wrong_t':   b'{"update_available":true,"version":123,"download_url":null,"message":[]}',
    'missing':   b'{"update_available":true}',
    'bad_json':  b'{not json',
    'empty':     b'{}',
    'array':     b'[1,2,3]',
    'big_str':   b'{"update_available":true,"version":"' + b'x'*4096 + b'","download_url":"","message":""}',
    'unicode':   '{"update_available":true,"version":"1.\u00e9.3","download_url":"https://updates.tyndomyn.net/\u26a0","message":"\u4e2d\u6587"}'.encode('utf-8'),
    'nested':    b'{"update_available":true,"version":{"nested":1},"download_url":"","message":""}',
    'null_emb':  b'{"update_available":true,"version":"1\\u00002","download_url":"","message":""}',
}
for n, b in seeds_json.items():
    with open(os.path.join(base, 'corpus/update_json', n), 'wb') as f:
        f.write(b)

seeds_cd = {
    'exact':     b'Spooky View - already running\x00',
    'no_nul':    b'Spooky View - already running',
    'short':     b'Spooky View',
    'prefix':    b'Spooky View - already running EXTRA padding here',
    'wrong':     b'Something else entirely here padding',
    'long':      b'Spooky View - already running' + b'A'*100,
    'all_nul':   b'\x00'*32,
    'no_term':   b'Spooky View - already runninX',
}
for n, b in seeds_cd.items():
    with open(os.path.join(base, 'corpus/copydata', n), 'wb') as f:
        f.write(b)

print('seeded', sum(map(len, (seeds_url, seeds_json, seeds_cd))), 'files')
