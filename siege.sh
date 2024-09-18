for i in {1..10000}
do
  curl localhost:8080 > /dev/null 2> /dev/null &
  curl localhost:8081 > /dev/null 2> /dev/null &
  echo $i
done
