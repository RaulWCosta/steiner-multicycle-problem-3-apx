import pandas as pd
import plotly.graph_objects as go
import numpy as np

def plot_dataframe(df):
    # Plot table comparing exact with apx for each instance
    # table = go.Figure(data=[go.Table(
    #     header=dict(values=["Instance", "Exact Value", "Exact Time", "Apx Value", "Apx Time"]),
    #     cells=dict(values=[df['instance'], df['exact_val'], df['exact_time'], df['apx_val'], df['apx_time']])
    # )])
    # table.show()

    # Plot distribution of apx - exact columns
    diff = df['apx_val'] / df['exact_val']
    histogram = go.Figure(data=[go.Histogram(x=diff, nbinsx=10)])
    histogram.update_layout(title_text="Distribution of optimality GAP", xaxis_title="Optimality GAP", yaxis_title="Frequency")
    histogram.show()


# def set_group_based_on_condition(instance_value):
#     if instance_value.startswith("m"):
#         return "m-pdtsp"
#     elif instance_value
    
#     else:
#         return None

# def preprocess_data(df):
    
#     df['instance'] = df['instance'].apply(lambda x:  x.split("/")[-1].split(".")[0])
    
#     return df

def separate_instances_types(df):

    # Boolean index to select rows that meet the condition
    condition = df['instance'].str.startswith("../../../testInst/m")

    # Split the DataFrame into two new DataFrames based on the condition
    df_starting_with_m = df[condition]
    df_not_starting_with_m = df[~condition]

    return df_starting_with_m, df_not_starting_with_m

def generate_table_row(input_df, classes="m-PDTSP"):
    
    df = pd.DataFrame([input_df.drop(columns=['instance']).mean()])
    df["Classes"] = classes
    df["num Inst"] = input_df.shape[0]
    df["GAP (\%)"] = (((df["apx_val"] / df["exact_val"]) - 1) * 100).round(2)
    df["APX time (s)"] = (df["apx_time"] / 1000).round(2)
    df["exact solver time (s)"] = (df["exact_time"] / 1000).round(2)

    return df[["Classes","num Inst", "GAP (\%)", "APX time (s)","exact solver time (s)"]]

def extract_type_2_rows(df_type_2, filter_str, filter_include: bool = True):
    
    #     # Group by the "instance" column containing "1x1", "2x2", ..., "5x5"
    # groups = df_not_starting_with_m.groupby(df_not_starting_with_m['instance'].str.contains('1x1|2x2|3x3|4x4|5x5'))

    # # Compute the mean of the numerical columns for each group
    # mean_values = groups.mean()
    
    # rows = []
    
    if filter_include:
        group = df_type_2[df_type_2["instance"].str.contains(filter_str)]
    else:
        group = df_type_2[~df_type_2["instance"].str.contains(filter_str)]
    row = generate_table_row(group, classes=filter_str)
    row["Classes"] = filter_str
    return row

if __name__ == "__main__":
    # Assuming your DataFrame is stored in a CSV file called 'data.csv'

    data_path = '_result_1.csv'
    df_raw = pd.read_csv(data_path, delimiter=';')

    # df = preprocess_data(df_raw)

    df_type_1, df_type_2 = separate_instances_types(df_raw)
    
    rows = []
    
    rows.append(generate_table_row(df_type_1))
    
    classes_frames = [
        "1x1",
        "2x2",
        "3x3",
        "4x4",
        "5x5",
        "W0.",
        "W0.1",
        "W0.2",
        "W0.3",
        "W0.4",
        "rg-016",
        "rg-032",
        "rg-064",
        "rg-128",
        "rg-256",
    ]
    for c in classes_frames:
        row = extract_type_2_rows(df_type_2, c, c != "W0.")
        if c == "W0.":
            row["Classes"] = "W0.0"
        if row["GAP (\%)"].iloc[0] <= 0:
            row["GAP (\%)"] = 0.0
        rows.append(row)
            
    df_result = pd.concat(rows)
    # generate_table_row(df_type_1)

    

    with open("latex_table.txt", "w", encoding="utf-8") as f:
        f.write(df_result.to_latex(index=False, float_format="{:.2f}".format))

    # Call the function to plot the data
    plot_dataframe(df_raw)
