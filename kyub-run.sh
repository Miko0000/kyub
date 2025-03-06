#!/bin/sh
while ! "$?"; do
	./build/Kyub $1
done