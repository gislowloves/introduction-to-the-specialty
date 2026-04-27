import json
import subprocess
from pathlib import Path

from flask import Flask, jsonify, render_template

app = Flask(__name__)

BASE_DIR = Path(__file__).resolve().parent
GAME_BINARY = BASE_DIR / "game"
CHARACTERS = [
    "Krosh",
    "Yozhik",
    "Nyusha",
    "Barash",
    "Losyash",
    "Pin",
    "Sovunya",
    "Kar-Karych",
    "Kopatych",
]


def make_initial_state():
    return {
        "day": 0,
        "stats": {name: 25 for name in CHARACTERS},
        "village": 50,
    }


state = make_initial_state()


def run_cpp(payload: dict):
    result = subprocess.run(
        [str(GAME_BINARY)],
        input=json.dumps(payload),
        capture_output=True,
        text=True,
        cwd=BASE_DIR,
    )

    if result.returncode != 0:
        return {
            "error": "cpp process failed",
            "returncode": result.returncode,
            "stderr": result.stderr,
        }

    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError:
        return {
            "error": "invalid json from cpp",
            "raw": result.stdout,
            "stderr": result.stderr,
        }


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/event/<int:day>")
def event(day):
    state["day"] = day

    payload = {
        "mode": "event",
        "day": day,
        "state": state,
    }

    data = run_cpp(payload)
    return jsonify(data)


@app.route("/choice/<int:day>/<int:choice>")
def choice(day, choice):
    global state

    payload = {
        "mode": "choice",
        "day": day,
        "choice": choice,
        "state": state,
    }

    data = run_cpp(payload)

    if "state" in data:
        state = data["state"]
    elif "stats" in data and "village" in data:
        state = {
            "day": day + 1,
            "stats": data["stats"],
            "village": data["village"],
        }

    return jsonify(data)


@app.route("/stats")
def stats():
    return jsonify(state)


@app.route("/restart", methods=["POST"])
def restart():
    global state

    state = make_initial_state()
    return jsonify(state)


if __name__ == "__main__":
    app.run(debug=True)
