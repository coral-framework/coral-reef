--------------------------------------------------------------------------------
-- Lua Archive File
-- Saved on Tue Jun 12 17:18:06 2012.
--------------------------------------------------------------------------------

-- Metadata
format_version = 1

-- Archived Objects

objects[1] = Object "dom.Company" {
	company = Service "dom.ICompany" {
		employees = ArrayOf "dom.IEmployee" {
			Ref{ id = 2, facet = 'employee' },
			Ref{ id = 3, facet = 'employee' },
			Ref{ id = 4, facet = 'employee' },
			Ref{ id = 5, facet = 'employee' },
		},
	},
	ceo = Ref{ id = 6, facet = 'employee' },
}

objects[2] = Object "dom.Employee" {
	employee = Service "dom.IEmployee" {
		leading = nil,
		working = ArrayOf "dom.IProject" {
			Ref{ id = 7, facet = 'product' },
		},
		name = "Joseph Java Newbie",
		role = "Developer",
		salary = 1000,
	},
}

objects[3] = Object "dom.Employee" {
	employee = Service "dom.IEmployee" {
		leading = nil,
		working = ArrayOf "dom.IProject" {
			Ref{ id = 7, facet = 'product' },
		},
		name = "Michael CSharp Senior",
		role = "Developer",
		salary = 4000,
	},
}

objects[4] = Object "dom.Employee" {
	employee = Service "dom.IEmployee" {
		leading = nil,
		working = ArrayOf "dom.IProject" {
			Ref{ id = 8, facet = 'service' },
		},
		name = "John Cplusplus Experienced",
		role = "Developer",
		salary = 5000,
	},
}

objects[5] = Object "dom.Employee" {
	employee = Service "dom.IEmployee" {
		leading = nil,
		working = ArrayOf "dom.IProject" {
			Ref{ id = 8, facet = 'service' },
		},
		name = "Jacob Lua Junior",
		role = "Developer",
		salary = 4000,
	},
}

objects[6] = Object "dom.Employee" {
	employee = Service "dom.IEmployee" {
		leading = Ref{ id = 8, facet = 'service' },
		working = ArrayOf "dom.IProject" {
		},
		name = "newCEO",
		role = "CEO",
		salary = 1000000,
	},
}

objects[7] = Object "dom.Product" {
	product = Service "dom.IProduct" {
		name = "Software2.0",
		value = 1000000,
	},
}

objects[8] = Object "dom.Service" {
	service = Service "dom.IService" {
		monthlyIncome = 60000,
		name = "Software1.0 Maintenance",
	},
}
