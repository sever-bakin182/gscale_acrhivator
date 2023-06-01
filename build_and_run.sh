#!/bin/bash

# Очистка предыдущих сборок
rm -rf build

# Сборка lib_gscale
cd lib/gscalelib
qmake
make
cd ../..

# Сборка archivergs
cd ui
qmake
make
cd ..

# Установка пути к папке lib/gscalelib/build в переменную окружения
export LD_LIBRARY_PATH=./lib/gscalelib/build

# Запуск приложения
./ui/build/archivergs
