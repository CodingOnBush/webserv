for i in {1..200}
do
  curl http://localhost:8082/cgi-bin/test.py > /dev/null 2> /dev/null &
  curl http://localhost:8082/cgi-bin/test.py > /dev/null 2> /dev/null &
  curl http://localhost:8082/cgi-bin/test.py > /dev/null 2> /dev/null &
  curl http://localhost:8082/cgi-bin/test.py > /dev/null 2> /dev/null &
  echo $i
done
