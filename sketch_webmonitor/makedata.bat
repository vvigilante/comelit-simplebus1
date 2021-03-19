@echo off
set MINIFY=%APPDATA%\Python\Python39\Scripts\css-html-js-minify.exe

cp monitor.html mtmp.html
%MINIFY% mtmp.html --overwrite
echo static const char index_html[] PROGMEM =  R^"EOF(>data.h
cat mtmp.html >> data.h
echo. >>data.h
echo )EOF^";>>data.h
del mtmp.html

cp wsworklet.js mtmp.js
%MINIFY% mtmp.js --overwrite
echo static const char wsworklet_js[] PROGMEM =  R^"EOF(>>data.h
cat mtmp.js >> data.h
echo. >>data.h
echo )EOF^";>>data.h
del mtmp.js

cp cq.js mtmp.js
%MINIFY% mtmp.js --overwrite
echo static const char cq_js[] PROGMEM =  R^"EOF(>>data.h
cat mtmp.js >> data.h
echo. >>data.h
echo )EOF^";>>data.h
del mtmp.js