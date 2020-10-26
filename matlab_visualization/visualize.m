%T = 

iteration = 0;

while 1
    fileName = strcat('points_', num2str(iteration), '.csv');
    if ~isfile(fileName)
       break
    end
    table = table2array(readtable(fileName));
    sortrows(table, 1)
    pointsLabel = table(:, 1);
    pointsX = table(:, 2);
    pointsY = table(:, 3);
   
    fileName = strcat('centroids_', num2str(iteration), '.csv');
    if ~isfile(fileName)
       break
    end
    table = table2array(readtable(fileName));
    sortrows(table, 1)
    centroidsLabel = table(:, 1);
    centroidsX = table(:, 2);
    centroidsY = table(:, 3);
   
    title(strcat('Iteration ', num2str(iteration)));
    hold on
    gscatter(pointsX,pointsY,pointsLabel)
    gscatter(centroidsX, centroidsY, centroidsLabel, "", "x", 10)
    hold off
    
    input("Press enter to view next iteration")
    
    iteration = iteration + 1;
end

title(strcat('Iteration ', num2str(iteration - 1), ' (final)'))
