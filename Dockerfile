FROM ubuntu:20.04

WORKDIR /SERVER

RUN apt-get update && apt-get install nlohmann-json3-dev -y make g++ curl

COPY . /SERVER/

RUN make 
CMD [ "./my_server" ]

EXPOSE 8000