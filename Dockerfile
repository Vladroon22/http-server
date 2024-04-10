FROM ubuntu:20.04

WORKDIR /SERVER

RUN apt-get update && apt-get install -y make 
RUN apt install -y g++ 

COPY . /SERVER/

RUN make 
CMD [ "./my_server" ]

EXPOSE 8000