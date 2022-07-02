#export DEBUG="afnet-server.js:(debug|info)"
which node || (which yum && sudo yum install node)
which node || (which apt && sudo apt install nodejs)
which npm || (which yum && sudo yum install npm)
npm install .
export DEBUG="afnet-server.js:*"
which node && node server.js
which node || nodejs server.js
