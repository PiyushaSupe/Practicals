If there is no c++ compiler(Mingw64 is ok too, do the install if Mingw is 32):

1. Download msys https://www.msys2.org/ -> msys2-x86_64-20260322.exe
2. Run the installer.
3. Run Msys lul
4. Run `pacman -Syu`
5. Say Y to everything nigga
6. Isko run kr na bacchi `pacman -S mingw-w64-ucrt-x86_64-gcc`
7. Window ko hata na bhidu
8. UCRT cha path uchal na bhai `C:\msys64\ucrt64\bin`
9. Right click on This PC
10. Go to properties
11. Click on Advanced system settings
12. Click on environment variables under 'Advanced Tab'
13. Under system variables edit 'Path' and add the copied path `C:\msys64\ucrt64\bin`
14. Test in cmd whether g++ is installed or not `g++ -fopenmp`

else start here:

15. Execute this in vscode terminal (first open the git cloned folder in vscode)
    `g++ -fopenmp .\hpc\hpc_prac1.cpp -o hpc_pract1`
16. Execute your program .\hpc_pract1.exe

To install dl packages:-
!pip install tensorflow pandas numpy matplotlib scikit-learn seaborn
versions:-
!pip install tensorflow==2.15.0 numpy==1.26.4 pandas==2.2.2 matplotlib==3.8.4 scikit-learn==1.4.2 seaborn==0.13.2 pillow==10.3.0 opencv-python==4.9.0.80 notebook==7.2.0

if python version issue ie tensorflow not working:

# 1. Create TensorFlow environment with Python 3.10

conda create -n tf_env python=3.10 -y

# 2. Activate environment

conda activate tf_env

# 3. Install TensorFlow

pip install tensorflow==2.15.0

# 4. Install Jupyter kernel support

pip install ipykernel

# 5. Add this environment as Jupyter kernel

python -m ipykernel install --user --name=tf_env

# 6. Start Jupyter Notebook

jupyter notebook

Then inside Jupyter Notebook:

Kernel → Change Kernel → tf_env
