#####Master file woth the value for eveyr variable in the different scripts
import genUtils as gu #The python swith is defined here

####Variables related with the filters

def get_param_value(var_name):
    for case in gu.switch(var_name):
                if case('number_of_filters'):
                        value = 2
                        break
                if case('EM_Iterations'):
                        value = 10
                        break
                if case('EM_latent_variables'):
                        value=1
                        break
                if case('PLS_Components'):
                        value = 30
                        break
                if case('Training_percentage'):
                        value=0.8   
                        break
                if case('TrainSamp'):
                        value=2
                        break
                if case('number_channels'):
                        value=64
                        break
                if case('IMM_Training_percentage'):
                        value=1
                        break
                if case('pca_components'):
                        value=300
                        break
                if case('ica_components'):
                        value=50
                        break
                if case():
                        print 'No value'
    return value

