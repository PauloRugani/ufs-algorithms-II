# Porto

- This program checks container records to find differences between registered and inspected data.

## Each record includes:

- Container code
- Importer CNPJ
- Net weight (kg)

## A container is selected for inspection if:

- The CNPJ is different, or
- The weight difference is greater than 10%.

## nspection priority:

- CNPJ mismatch
- Highest weight difference (%)

## Execution Limit

 - Maximum execution time: **1 second**

## Input
    [number of registered containers]
    [Code] [CNPJ] [Weight]
    ...
    [number of inspected containers]
    [Code] [CNPJ] [Weight]
    ...

## Output
    [Code]:[old CNPJ]<->[new CNPJ]
    [Code]:[difference]kg([percentage]%)

## Example

### Input

    6
    QOZJ7913219 34.699.211/9365-11 13822
    FCCU4584578 50.503.434/5731-28 16022
    KTAJ0603546 20.500.522/6013-58 25279
    ZYHU3978783 43.172.263/4442-14 24543
    IKQZ7582839 51.743.446/1183-18 12160
    HAAZ0273059 25.699.428/4746-79 16644
    5
    ZYHU3978783 43.172.263/4442-14 29765
    IKQZ7582839 51.743.446/1113-18 18501
    KTAJ0603546 20.500.522/6113-58 17842
    QOZJ7913219 34.699.211/9365-11 16722
    FCCU4584578 50.503.434/5731-28 16398


### Output

    KTAJ0603546:20.500.522/6013-58<->20.500.522/6113-58
    IKQZ7582839:51.743.446/1183-18<->51.743.446/1113-18
    QOZJ7913219:2900kg(21%)
    ZYHU3978783:5222kg(21%)