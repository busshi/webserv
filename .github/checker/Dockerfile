FROM		ubuntu:latest

RUN			apt update && apt install -y clang make

WORKDIR		/.github/checker

ENTRYPOINT	[ "make" ]
