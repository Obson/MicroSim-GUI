# MicroSim

**MicroSim** is a micro-economic modelling program designed to complement SimX (which is a *macro*-economic modelling system). Like SimX, it provides its own graphics, but it will also (eventually) output its results in the form of a CSV file which can be read into a spreadsheet for further analysis.


## Components of a Model and Their Interactions ##

A model contains the following components:

* Workers
* Firms
* A Government (later versions may allow more thn one Government)
* Banks

These components interact in fairly simple ways in response to 

* the passing of time
* the flow of money

**Time** is measured in *periods* (AKA *cycles* or *iterations*). These do not correspond in any precise or consistent way to time in the real world. Thus, for example, in the model all firms pay their employees once per period, while in the real world wages and salaries may be paid weekly or monthly, or sometimes four weekly or at varying intervals. This is one of the limitations of the modelling system.

**Money** in MicroSim is curently dimensionless and may be thought of as arbitrary *currency units* or CUs. This may change when we add foreign exchange (later).

Every component is *triggered* once per period, and when triggered it will do something depending on its circumstances at the time. A firm, for example, may hire or fire employees, and a worker may make some purchases. The parameters of the model determine more precisely how they respond. 

## How it Works ##
The model is entirely 'driven by' money. And as in the real world money emanates in the first instance from the government. There are a number of ways money may be disbursed by the government, for example:

* To businesses:
	- as a direct payment or grant
	- by purchasing goods or services
	- via banks as loans (see e.g. Mosler for a description of the of how governments finance loans issued by banks)
	- as interest payments on government bonds.
* To individuals:
	- as benefits (e.g. unemployment benefit)
	- as payment through a state-subsidised Job Guarantee Scheme


At this stage we will ignore the payment of interest on government bonds as this is really an irrelevance. 

### When the clock ticks... ###
The first thing that happens each time the clock ticks (i.e. at the beginning of each period) is that the government spends a certain amount of money 'into the economy'. One of the ways it does this in the real world is by supporting nationalised industries, the armed forces, the NHS, the civil service, parliament and the royal family (and no doubt a number of other things I don't know about or have overlooked). Another is by paying 'benefits' such as unemployment benefit to individuals.

The question then is how much the government should pay.

#### Government expenditure ####

**This section is under revision**

The amount the government needs to spend (ignoring benefits) is determined by the size of the economy, the wage rate, the tax rate, and the rate of investment in employment. In the real world these quantities, with the exception of the tax rate, are quite hard to determine. For MicroSim the situation is much simpler&mdash;they are all parameters given in the model definition. We give, for example, an actual population size and a target rate of employment. [This statement is inaccurate &mdash; MUST FIX: By multiplying these together MicroSim obtains the size of the economically active population. It can then work out how much money the government needs to spend in order to bring that number of workers into the economy.]

As we have already noted, there are a number of ways the government  can inject money into the economy. The most straightforward of these is simply by 'buying stuff'. But at the beginning of the economic process the only thing that is automatically available to buy is whatever is provided by the government. So to make this work the government effectively splits itself into two main parts: the Treasury, which creates the money, and the civil service (plus the NHS, the armed services, etc., etc.) which uses that money to pay the salaries of its employees. In effect the Treasury 'buys services' from its commercial arm, the civil service (etc.). The civil service's employees can then use their salaries to buy whatever they want, either from their employer&mdash;the government-owned businesses, or from private businesses, should there happen to be any, with the result that the money created by the Treasury gets dispersed into the system.

Implausibly elegant though this may sound, it works!

#### Benefits ####
Having transferred the required sum into the government owned businesses the government next pays benefits to all out-of-work individuals. Again, the amount of benefts paid is determined by a parameter in the model definition file. The effect of these payments on the size of the economy is not currently taken into account in calculating the level of government expenditure required. It probably should be...

#### Businesses ####

...

## Parameters of a Model ##

![Parameter Wizard](http://obson.net/wp-content/uploads/2017/08/parameter-wizard.jpeg)

