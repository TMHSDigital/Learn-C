all:
	gcc main.c -o learn.exe

clean:
	del /f learn.exe 2>nul || true 