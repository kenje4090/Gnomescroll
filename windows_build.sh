

rm -rf ./dist

mkdir ./dist
mkdir ./dist/netclient
mkdir ./dist/server

cp -f ./server/dats/cube_dat.py ./netclient/cube_dat.py

cd netclient
./win_package.sh
cd ..
cp -rf ./netclient/dist/. ./dist/netclient/

cd server
./win_package.sh
cd ..
cp -rf ./server/dist/. ./dist/server/

rm -rf /c/Documents\ and\ Settings/Administrator/Desktop/Dropbox/gnomescroll_
cp -rf ./dist/ /c/Documents\ and\ Settings/Administrator/Desktop/Dropbox/gnomescroll_
