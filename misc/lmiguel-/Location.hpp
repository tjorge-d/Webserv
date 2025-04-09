/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 15:48:27 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/04/03 14:41:25 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <Parser.hpp>

class Location{

	public:

	Location ( void );
	~Location ( void );

	void	setAutoindexState ( bool state);
	void	setAllowedServices ( std::string allowed_services );
	void	setRootDirectory ( std::string root );
	void	setIndexFile ( std::string file);

	private:

	bool			_autoindexState;
	std::string		_allowedServices;
	std::string		_rootDirectory;
	std::string		_indexFile;
};

#endif