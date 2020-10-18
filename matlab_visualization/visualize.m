%T = 

iteration = 0;

while 1
    fileName = strcat('points_', num2str(iteration), '.csv');
    if ~isfile(fileName)
       break
    end
    
    table = table2array(readtable(fileName));
    sortrows(table, 1)
    x = table(:, 2);
    y = table(:, 3);
    cluster = table(:, 1);
    gscatter(x,y,cluster)
    
    input("Press enter to view next iteration")
    
    iteration = iteration + 1;
end

disp("All")
disp(iteration);