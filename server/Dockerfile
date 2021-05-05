FROM python:3-slim as base

FROM base as builder

RUN pip install pipenv
RUN apt-get update && apt-get install -y --no-install-recommends git gcc build-essential

COPY Pipfile* ./
RUN PIPENV_VENV_IN_PROJECT=1 pipenv install --deploy

FROM base as runtime

# Copy virtual env from python-deps stage
COPY --from=builder /.venv /.venv
ENV PATH="/.venv/bin:$PATH"

# Create and switch to a new user
RUN useradd --create-home appuser
WORKDIR /home/appuser
USER appuser

# Install application into container
COPY . .

# Run the application
ENTRYPOINT [ "python", "./LORA_PI_Rx.py" ]
