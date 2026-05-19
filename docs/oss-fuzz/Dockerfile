FROM gcr.io/oss-fuzz-base/base-builder
RUN apt-get update && apt-get install -y --no-install-recommends zip && rm -rf /var/lib/apt/lists/*
RUN git clone --depth 1 https://github.com/SchwartzKamel/spooky-view.git $SRC/spooky-view
WORKDIR $SRC/spooky-view
COPY build.sh $SRC/
