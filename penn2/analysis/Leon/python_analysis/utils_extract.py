#This function extracts the relevant information from the metadata file
def extract_data(filename):
    inFile = open(filename)
    buffer_line = []
    keepCurrentSet = True
    band_dict = {}
    dict_idx = 1
    date_str = 'Null';
    first_channel = 0;
    for line in inFile:
        line_iter = line.split(':')
        print line_iter
        if line_iter[0][:4] == 'Band': #If we find a band field
            band_dict[dict_idx] = line_iter[1].strip()
            dict_idx += 1;
        if line_iter[0] == 'Date of the experiment': #If we find a band field
            date_str = line_iter[1].strip()
        if line_iter[0] == 'First Channel': #If we information about the first channel
            first_channel = int(line_iter[1].strip())
        
    inFile.close()
    
    return date_str, band_dict, first_channel
    
