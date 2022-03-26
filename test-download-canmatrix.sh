#!/bin/bash

main () {
    mkdir -p canmatrix
    pushd . >/dev/null
    cd canmatrix
    while read line; do
        local files=( ${files[@]} $line )
    done << EOF
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g03_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g02_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g01_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g06_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g07_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g08_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g11_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g10_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g09_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g14_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g15_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/g/canmatrix2_092g16_tif.zip

https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j03_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j02_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j01_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j06_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j07_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/j/canmatrix2_092j08_tif.zip

https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h04_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h03_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h05_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h06_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h12_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h11_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h13_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/h/canmatrix2_092h14_tif.zip

https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/i/canmatrix2_092i03_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/i/canmatrix2_092i04_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/i/canmatrix2_092i05_tif.zip
https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/50k_tif/092/i/canmatrix2_092i06_tif.zip
EOF
    for i in "${files[@]}"; do
        echo "== Downloading $i"
        wget "$i"
        # long command line so not using wget's buildin feature
        sleep $(( ($RANDOM % 3) + 3 ))
    done
    popd >/dev/null
}
time main "$@"
