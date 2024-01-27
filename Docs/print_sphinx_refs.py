import pickle
import os
dirname = os.path.dirname(__file__)

envfile = open(f'{dirname}/build/doctrees/environment.pickle', 'rb')
environ = pickle.load(envfile)
elabels = environ.domaindata['std']['labels']

for k,v in elabels.items():
    # docname, (nodeid, title)
    print(f"{k}: {v}")