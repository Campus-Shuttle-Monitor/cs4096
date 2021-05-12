FROM arm32v5/python:3 as builder

RUN pip install pipenv

COPY Pipfile* ./
RUN PIPENV_VENV_IN_PROJECT=1 pipenv install --deploy

FROM arm32v5/python:3-slim as runtime

# Copy virtual env from python-deps stage
COPY --from=builder /.venv /.venv
ENV PATH="/.venv/bin:$PATH"

# Install application into container
COPY . .

# Run the application
ENTRYPOINT [ "python", "./LORA_PI_Rx.py" ]
