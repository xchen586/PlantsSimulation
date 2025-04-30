import pandas as pd
#import matplotlib.pyplot as plt

source_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo\\12_4_2_regions_info_test.csv'
destination_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo\\12_4_2_regions_info_test.csv'
# open csv file
def open_csv(file_path):
    """
    Open a CSV file and return a DataFrame.
    """
    try:
        df = pd.read_csv(file_path)
        return df
    except Exception as e:
        print(f"Error opening file: {e}")
        return None

def post_process_regions_info_csv(file_path, dest_path):
    """
    Post-process the DataFrame.
    """
    # describe the dataframe
    # open the file
    df = open_csv(file_path)
    # set all "type 1 " to "Unknown"
    df['type 1'] = 'Unknown'

    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] >= 15), 'type 1'] = 'Temperate'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] > 40), 'type 1'] = 'Humid'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] < 15), 'type 1'] = 'Dry'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] < 5), 'type 1'] = 'Desert'
    df.loc[(df['MinHeight'] > 2500) & (df['AvgHumidity'] > 40), 'type 1'] = 'Tundra'
    df.loc[df['MinHeight'] > 4000, 'type 1'] = 'Frozen'
    df.loc[(df['MinHeight'] < 20) & (df['NearSea'] == 1.0), 'type 1'] = 'Ocean'
    # set all "type 1 " to "Unknown"
    df['type 1'] = 'Unknown'

    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] >= 15), 'type 1'] = 'Temperate'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] > 40), 'type 1'] = 'Humid'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] < 15), 'type 1'] = 'Dry'
    df.loc[(df['MinHeight'] > 0) & (df['AvgHumidity'] < 5), 'type 1'] = 'Desert'
    df.loc[(df['MinHeight'] > 2500) & (df['AvgHumidity'] > 40), 'type 1'] = 'Tundra'
    df.loc[df['MinHeight'] > 4000, 'type 1'] = 'Frozen'
    df.loc[(df['MinHeight'] < 20) & (df['NearSea'] == 1.0), 'type 1'] = 'Ocean'

    print(df.groupby('type 1').count())

    print(df[df['type 1'] == 'Unknown'].describe())

    df['level'] = 0

    df['level'] = (df['RegionId'] % 5 + 1)
    df.loc[df['type 1'] == 'Ocean', 'level'] = 1
    df.loc[(df['level'] == 1) & (df['type 1'] == 'Ocean') & (df['RegionId'] % 3 == 0), 'level'] = 2

    # show histogram for level
    #plt.hist(df['level'], bins=5)
    #plt.title('Level Distribution')
    #plt.xlabel('Level')
    #plt.ylabel('Count')
    #plt.xticks([0, 1, 2, 3, 4], ['1', '2', '3', '4', '5'])
    #plt.show()

    # group by name, count
    grouped = df.groupby('Name').count()

    # replaced duplicate "Name" values by "Unknown"
    df.loc[df['Name'].duplicated(), 'Name'] = 'Unknown'
    # group by name, count
    grouped = df.groupby('Name').count()

    # count how many regions have "Unknown" name
    unknown_count = df[df['Name'] == 'Unknown'].count()

    # load name dataframe from "namedb.csv"
    namedb = pd.read_csv('namedb.csv', delimiter=',')
    print(namedb.describe())

    # drop duplicated names
    namedb = namedb.drop_duplicates(subset=['Name'])
    print(namedb.describe())

    # rename type 1 column as "Type"
    df.rename(columns={'type 1': 'Type'}, inplace=True)

    # show piechart for type 1

    # count the number of each type
    counts = df['Type'].value_counts()

    # create a pie chart
    #plt.pie(counts, labels=counts.index, autopct='%1.1f%%')
    #plt.title('Type 1 Distribution')
    #splt.show()

    df_regions = df
    df_names_shuffled = namedb

    df_regions['TypeCount'] = df_regions.groupby('Type').cumcount()
    df_names_shuffled['TypeCount'] = df_names_shuffled.groupby('Type').cumcount()

    # Merge DataFrames on Type and the incremental count
    df_result = pd.merge(df_regions.drop('Name', axis=1),
                        df_names_shuffled,
                        on=['Type', 'TypeCount'],
                        how='left')

    # Drop the temporary count column
    df_result = df_result.drop('TypeCount', axis=1)

    df = df_result

    # group by name, count
    grouped = df.groupby('Name').count()

    # replaced duplicate "Name" values by "Unknown"
    df.loc[df['Name'].duplicated(), 'Name'] = 'Unknown'
    # group by name, count
    grouped = df.groupby('Name').count()

    # count how many regions have "Unknown" name
    unknown_count = df[df['Name'] == 'Unknown'].count()
    print(f"Number of regions with unknown name: {unknown_count['RegionId']}")

    # move the "Name" column to be after the "Type" column
    # get the columns of the dataframe
    columns = df.columns.tolist()
    col_to_move = columns.pop(columns.index('Name'))  # remove 'Name' from the list
    columns.insert(columns.index('Type') + 1, col_to_move)  # insert 'Name' after 'Type'
    df = df[columns]  # reorder the DataFrame
    
    # save the dataframe to a csv file
    df.to_csv(dest_path, index=False)

post_process_regions_info_csv(source_path, destination_path)