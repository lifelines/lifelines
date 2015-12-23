for fi in lifelines*
do
    if [ -f "$fi" ]
    then
      echo -ne "$ md5sum $fi\r\n"
      md5sum $fi
    fi
done

