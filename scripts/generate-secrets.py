import json
import os

output = {
    "auth_url": "https://oauth.yandex.ru/authorize",
    "access_token_url": "https://oauth.yandex.ru/token?grant_type=authorization_code",
    "client_id": os.environ.get("YANDEX_CLIENT_ID", ""),
    "client_secret": "",
    "redirect_base": "http://127.0.0.1",
    "redirect_port": 1337,
    "scopes": [
        "iot:view",
        "iot:control",
        "login:info",
        "login:avatar",
        "login:email"
    ]
}

os.makedirs("resources/auth", exist_ok=True)

with open("resources/auth/secrets.json", "w", encoding="utf-8") as f:
    json.dump(output, f, indent=2, ensure_ascii=False)
