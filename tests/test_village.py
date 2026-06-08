from unittest.mock import patch
import pytest
from app import app, make_initial_state

@pytest.fixture
def client():
    app.config["TESTING"] = True
    with app.test_client() as client:
        yield client

def test_make_initial_state_day_is_zero():
    """В начале игры день должен быть 0"""
    state = make_initial_state()
    assert state["day"] == 0

def test_make_initial_state_village_is_50():
    """Начальное настроение деревни — 50"""
    state = make_initial_state()
    assert state["village"] == 50

def test_make_initial_state_all_characters_present():
    """Все 9 персонажей должны быть в stats"""
    state = make_initial_state()
    expected = {"Krosh", "Yozhik", "Nyusha", "Barash",
                "Losyash", "Pin", "Sovunya", "Kar-Karych", "Kopatych"}
    assert set(state["stats"].keys()) == expected

def test_make_initial_state_all_stats_are_25():
    """Все начальные характеристики персонажей — 25"""
    state = make_initial_state()
    for character, value in state["stats"].items():
        assert value == 25, f"{character} должен начинать с 25, а не с {value}"

def test_home_returns_200(client):
    """GET / должен возвращать статус 200"""
    response = client.get("/")
    assert response.status_code == 200

def test_restart_resets_state(client):
    """POST /restart должен вернуть начальное состояние"""
    response = client.post("/restart")
    assert response.status_code == 200
    data = response.get_json()
    assert data["day"] == 0
    assert data["village"] == 50

def test_restart_resets_all_stats(client):
    """После рестарта все характеристики персонажей снова 25"""
    response = client.post("/restart")
    data = response.get_json()
    for character, value in data["stats"].items():
        assert value == 25

def test_stats_after_restart(client):
    """После рестарта /stats должен отражать начальное состояние"""
    client.post("/restart")
    response = client.get("/stats")
    data = response.get_json()
    assert data["day"] == 0
    assert data["village"] == 50