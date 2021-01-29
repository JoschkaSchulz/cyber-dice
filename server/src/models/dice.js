import { DataTypes } from 'sequelize';

export default (sequelize) => {
  const Dice = sequelize.define('Dice', {
    username: DataTypes.STRING,
  }, {
    classMethods: {},
  });

  return Dice;
};
