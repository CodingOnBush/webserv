for i in {1..300}
do
  curl localhost:8080 > /dev/null 2> /dev/null &
  echo $i
done
