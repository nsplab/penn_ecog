function yfit = crossfun(xtrain,ytrain,xtest,rbf_sigma,boxconstraint)
%function to fed to the cross validationcode

% Train the model on xtrain, ytrain, 
% and get predictions of class of xtest
svmStruct = svmtrain(xtrain,ytrain,'Kernel_Function','rbf',...
   'rbf_sigma',rbf_sigma,'boxconstraint',boxconstraint);
yfit = svmclassify(svmStruct,xtest);

%svmCl = svmtrain(dataset, labels_dataset,'Kernel_Function','rbf',...
    %'boxconstraint', Inf, 'tolkkt',1e-3);