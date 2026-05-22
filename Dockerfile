FROM python:3.12-slim AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY camomile.cpp .

RUN g++ -std=c++17 -O2 camomile.cpp -o game

FROM python:3.12-slim

WORKDIR /app

COPY requirements.txt .

RUN pip install -r requirements.txt

COPY app.py .
COPY templates/ templates/
COPY static/ static/

COPY --from=builder /app/game .

EXPOSE 5000

CMD ["python", "app.py"]