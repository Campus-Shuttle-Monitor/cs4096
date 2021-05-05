FROM python:3 as builder

COPY requirements.txt .

RUN pip install --user -r requirements.txt

FROM python:3-alpine

COPY --from=builder /root/.local /root/.local

COPY . .

CMD [ "python", "./LORA_PI_Rx.py" ]
