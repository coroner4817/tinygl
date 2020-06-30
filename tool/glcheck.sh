FILE=$1
OPT=$2
if [ "$OPT" != "-a" ] && [ "$OPT" != "-m" ] || test -z "$FILE"; then
  echo "usage: $0 [-a, -m] <file>"
  exit 0
fi

if [ "$OPT" = "-a" ]; then
  echo "appending glcheck"
  sed -i -e '/gl[^m].*;/s/$/glCheckError();/' $FILE
  egrep 'glCheckError()' $FILE | while read -r line ; do
    echo "Processing: $line" | GREP_COLOR='1;31' grep -E 'glCheckError' --color=always
  done
else
  echo "removing glcheck"
  sed -i 's/glCheckError();//g' $FILE
  egrep 'gl[^m].*;' $FILE | while read -r line ; do
    echo "Processing: $line"
  done
fi