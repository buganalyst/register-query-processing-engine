#!/usr/bin/env python3

import argparse
import json
import os
import subprocess
import sys
import threading
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse


PAGE = r"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Register Query Engine</title>
  <style>
    :root { --ink:#172033; --muted:#697386; --line:#dfe3eb; --blue:#3157d5; --paper:#fff; --wash:#f4f6fa; }
    * { box-sizing:border-box; }
    body { margin:0; min-height:100vh; background:var(--wash); color:var(--ink); font:14px/1.45 Inter,Segoe UI,sans-serif; }
    header { height:64px; display:flex; align-items:center; justify-content:space-between; padding:0 28px; background:#fff; border-bottom:1px solid var(--line); }
    header strong { font-size:16px; } header span { color:#5e6879; font-size:12px; }
    main { max-width:1180px; margin:22px auto; display:grid; grid-template-columns:minmax(0,1.1fr) minmax(320px,.9fr); gap:16px; padding:0 16px; }
    .card { background:var(--paper); border:1px solid var(--line); border-radius:10px; overflow:hidden; box-shadow:0 2px 10px rgba(22,32,51,.04); }
    .card-head { height:48px; display:flex; align-items:center; justify-content:space-between; padding:0 16px; border-bottom:1px solid var(--line); font-weight:700; }
    textarea { width:100%; height:310px; resize:vertical; border:0; outline:none; padding:18px; font:14px/1.6 Consolas,monospace; color:#25304a; }
    .actions { display:flex; gap:8px; padding:12px 16px; border-top:1px solid var(--line); background:#fbfcfe; }
    button { border:0; border-radius:7px; padding:9px 14px; background:var(--blue); color:white; font-weight:700; cursor:pointer; }
    button.secondary { background:#edf0f5; color:#40506a; }
    pre { margin:0; min-height:310px; max-height:520px; overflow:auto; white-space:pre-wrap; padding:18px; font:13px/1.55 Consolas,monospace; }
    .examples { display:flex; flex-wrap:wrap; gap:7px; margin-top:10px; }
    .examples button { background:white; border:1px solid var(--line); color:#4a5770; font-size:12px; padding:6px 8px; }
    .status { display:inline-flex; align-items:center; gap:7px; color:#3c7a57; } .status i { width:8px; height:8px; border-radius:50%; background:#38a169; }
    @media(max-width:850px){ main{grid-template-columns:1fr} textarea,pre{height:260px;min-height:260px} }
  </style>
</head>
<body>
<header><strong>Register Query Processing Engine</strong><span class="status"><i></i> Engine ready</span></header>
<main>
  <section class="card">
    <div class="card-head"><span>SQL Console</span><span id="shortcut">Ctrl + Enter</span></div>
    <textarea id="sql" spellcheck="false" placeholder="Enter SQL...">CREATE TABLE students (id INT, name TEXT)
INSERT INTO students VALUES (1, 'Arin')
INSERT INTO students VALUES (2, 'Preeti')
SELECT * FROM students</textarea>
    <div class="actions">
      <button id="run">Run query</button>
      <button class="secondary" id="clear">Clear</button>
      <button class="secondary" id="stats">STATS</button>
    </div>
  </section>
  <section class="card">
    <div class="card-head"><span>Output</span><span id="rows"></span></div>
    <pre id="output">Run a query to see results.</pre>
  </section>
</main>
<script>
const sql = document.getElementById('sql'), output = document.getElementById('output'), rows = document.getElementById('rows');
async function run() {
  output.textContent = 'Executing...'; rows.textContent = '';
  const response = await fetch('/api/execute', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({sql:sql.value})});
  const data = await response.json();
  output.textContent = data.output || data.error || 'No output';
  const match = (data.output || '').match(/(\d+) row\(s\) returned/);
  rows.textContent = match ? match[1] + ' rows' : '';
}
document.getElementById('run').onclick = run;
document.getElementById('clear').onclick = () => { sql.value = ''; output.textContent = 'Ready.'; rows.textContent = ''; sql.focus(); };
document.getElementById('stats').onclick = () => { sql.value = 'STATS'; run(); };
sql.addEventListener('keydown', e => { if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') run(); });
</script>
</body>
</html>"""


class Engine:
    def __init__(self, executable):
        self.condition = threading.Condition()
        self.buffer = ""
        self.process = subprocess.Popen(
            [executable],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=0,
        )
        self.reader = threading.Thread(target=self._read_output, daemon=True)
        self.reader.start()
        self._wait_ready()

    def _read_output(self):
        while True:
            char = self.process.stdout.read(1)
            if not char:
                return
            with self.condition:
                self.buffer += char
                self.condition.notify_all()

    def _wait_ready(self):
        with self.condition:
            self.condition.wait_for(lambda: "rqe> " in self.buffer, timeout=5)
            self.buffer = ""

    def execute(self, sql):
        lines = [line.strip() for line in sql.splitlines() if line.strip()]
        parts = []
        for line in lines:
            with self.condition:
                self.buffer = ""
            try:
                self.process.stdin.write(line + "\n")
                self.process.stdin.flush()
            except BrokenPipeError:
                raise RuntimeError("The database engine stopped unexpectedly")
            with self.condition:
                ok = self.condition.wait_for(lambda: "rqe> " in self.buffer, timeout=10)
                output = self.buffer
                if "rqe> " in output:
                    output = output.rsplit("rqe> ", 1)[0]
            if not ok:
                raise RuntimeError("The database engine did not return a result")
            parts.append(output.strip())
        return "\n".join(part for part in parts if part).strip()

    def close(self):
        if self.process.poll() is None:
            try:
                self.process.stdin.write("EXIT\n")
                self.process.stdin.flush()
            except BrokenPipeError:
                pass
            self.process.wait(timeout=2)


engine = None


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        return

    def send_json(self, status, payload):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if urlparse(self.path).path != "/":
            self.send_error(404)
            return
        body = PAGE.encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if urlparse(self.path).path != "/api/execute":
            self.send_error(404)
            return
        try:
            length = int(self.headers.get("Content-Length", "0"))
            payload = json.loads(self.rfile.read(length).decode("utf-8"))
            sql = payload.get("sql", "")
            if not sql.strip():
                self.send_json(400, {"error": "Enter a SQL query."})
                return
            self.send_json(200, {"output": engine.execute(sql)})
        except Exception as error:
            self.send_json(500, {"error": str(error)})


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--engine", required=True)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    if not os.path.isfile(args.engine):
        print("Engine executable not found: " + args.engine, file=sys.stderr)
        return 1

    global engine
    engine = Engine(args.engine)
    server = ThreadingHTTPServer((args.host, args.port), Handler)
    url = "http://{}:{}/".format(args.host, args.port)
    print("Register Query Engine GUI: " + url)
    webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        engine.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
