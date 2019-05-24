# script for running all files with *.in ending from data.
make
for name in ../data/*.in; do
    [ -f "$name" ] || break
    tmp="${name##*/}"
    file_name="${tmp%.*}"
    echo "$file_name"
    ./solve $file_name > ../log/$file_name.log
done
