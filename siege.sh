for i in {1..100}
do
  curl -X GET localhost:8080/logo.png?test=$i -i > /dev/null 2> /dev/null &
  echo $i
done