Publishing New Releases
=======================

First, update the version number throughout the repo and push the change:

    ./tools/makever.py --version X.Y.Z
    git commit -a -m "Update version"
    git push

GitHub CI Actions are used to automatically build a draft package whenever a tag is pushed to repo:

    git tag X.Y.Z
    git push origin X.Y.Z


This will generate a draft release with a bulleted-list of ``git`` changes.
Edit the list and title as desired, then use the ``Publish`` button on the web interface to publish the release

At this point, a GitHub Action will run that will download the new combined ``JSON`` file from the tag and update the ``global`` release with it.
