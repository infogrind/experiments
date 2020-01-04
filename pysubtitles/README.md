# pysubtitles

Python code to detect and download subtitles for movie files.

## Development

The file `requirements.txt` has been generated from a Python3 VirtualEnv
installation, that is, the necessary packages (principally subliminal) were
installed using `pip` in the VirtualEnv directory, and then `requirements.txt`
was installed using
```
pip freeze > requirements.txt
```

To create your own VirtualEnv for development, first install the `virtualenv`
tool using
```
pip install virtualenv
```
Then create and activate the virtual environment:
```
virtualenv venv  # venv is the path where it is created, choose anything
source venv/bin/activate  # maps all binary commands (python, pip, etc.)
                          # to the one of the virtualenv
```

Once the virtual environment is installed and activated, you can install the
necessary packages using
```
pip install -r requirements.txt
```

## Sources

* Couchpotato (contains Python code to determine subtitles in movie files)
	* Uses [Subliminal](https://subliminal.readthedocs.io/en/latest/) ("a
	  library to search and download subtitles"), apparently also supports
 	  downloading  from Opensubtitles, and checking video files for existing
          subtitles.
* VLSub (lua source code for illustration of opensubtitles API)
* Potentially https://github.com/agonzalezro/python-opensubtitles (existing
  Python wrapper for opensubtitles.org)
