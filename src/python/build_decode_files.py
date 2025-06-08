import pandas as pd

df = pd.read_excel('D:\CPU\Instructions.xlsx') #, sheet_name='Sheet1')
cell_value = df.loc[0, 'Description']
print(type(cell_value))
print(cell_value)

#for row in df.iterrows():
#    for cell in row:
#        print(cell(2))

#import xlrd

#workbook = xlrd.open_workbook('D:\CPU\Instructions.xlsx')
#worksheet = workbook.sheet_by_name('Sheet1')
#num_rows = worksheet.nrows - 1
#curr_row = -1
#while curr_row < num_rows:
#        curr_row += 1
#        value = worksheet.cell(curr_row, 2)
#        print(value)