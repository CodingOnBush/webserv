for i in {1..20000}
do
  curl -X GET localhost:8080/logo.png?test=$i -i > /dev/null 2> /dev/null &
  # curl localhost:8081 > /dev/null 2> /dev/null &
  echo $i
done
